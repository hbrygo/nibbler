#include "nibbler.hpp"
#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <string>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <random>

int currentLibrary = SDL3;

typedef void* (*create_t)(int, int, bool);
typedef void (*destroy_t)(void*);
typedef void (*display_t)(void*, const Game&);
typedef int (*input_t)(void*);

struct GameConfig {
    int width = 10;
    int height = 10;

    std::string library = "sdl3";

    bool michaelMode = false;
    bool despawnApple = false;

    bool multiplayer = false;
    bool online = false;
    bool host = false;

    std::string roomId;
};

int askChoice(const std::string& title, const std::vector<std::string>& options)
{
    int input;

    while (true) {
        std::cout << "\n=== " << title << " ===\n\n";

        for (size_t i = 0; i < options.size(); i++) {
            std::cout << i + 1 << ") " << options[i] << "\n";
        }

        std::cout << "\nReponse: ";
        std::cin >> input;

        if (!std::cin.fail() && input >= 1 && input <= (int)options.size())
            return input;

        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
}

int askInt(const std::string& label, int def)
{
    std::string s;
    std::cout << label << " (defaut " << def << "): ";
    std::getline(std::cin >> std::ws, s);
    return s.empty() ? def : std::stoi(s);
}

std::string askString(const std::string& label)
{
    std::string s;
    std::cout << label << ": ";
    std::getline(std::cin >> std::ws, s);
    return s;
}

std::string generateRoomId()
{
    static const char alphanum[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::string id;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, sizeof(alphanum) - 2);

    for (int i = 0; i < 6; i++)
        id += alphanum[dist(gen)];

    return id;
}

GameConfig runMenu()
{
    GameConfig c;

    int mainChoice = askChoice("GAME MODE", {
        "Multi",
        "Solo",
        "Mode_Michael"
    });

    if (mainChoice == 3) {
        c.michaelMode = true;
        c.library = "sdl3";
        c.width = 17;
        c.height = 32;

        std::cout << "Michael Mode Activated!\n";
        return c;
    }

    if (mainChoice == 2) {
        c.despawnApple = askChoice("Despawn Apple?", {"Yes", "No"}) == 1;
        c.width = askInt("Width", 10);
        c.height = askInt("Height", 10);

        int lib = askChoice("Library", {"sdl3", "sfml", "gl"});
        c.library = (lib == 1 ? "sdl3" : lib == 2 ? "sfml" : "gl");

        return c;
    }

    c.multiplayer = true;

    int multiChoice = askChoice("MULTI MODE", {
        "Local",
        "Online"
    });

    if (multiChoice == 1) {
        c.width = askInt("Width", 10);
        c.height = askInt("Height", 10);

        int lib = askChoice("Library", {"sdl3", "sfml", "gl"});
        c.library = (lib == 1 ? "sdl3" : lib == 2 ? "sfml" : "gl");

        c.online = false;
        return c;
    }

    c.online = true;

    int onlineChoice = askChoice("ONLINE MODE", {
        "Host",
        "Client"
    });

    if (onlineChoice == 1) {

        c.host = true;

        c.width = askInt("Width", 10);
        c.height = askInt("Height", 10);

        int lib = askChoice("Library", {"sdl3", "sfml", "gl"});
        c.library = (lib == 1 ? "sdl3" : lib == 2 ? "sfml" : "gl");

        c.roomId = generateRoomId();

        std::cout << "\n========================\n";
        std::cout << "ROOM CREATED\n";
        std::cout << "ID: " << c.roomId << "\n";
        std::cout << "========================\n";

        return c;
    }

    c.host = false;

    std::cout << "Waiting for room ID...\n";
    c.roomId = askString("Enter room ID");

    return c;
}

int main()
{
    int onAppleSound = 0;
    std::mutex onAppleMutex;
    std::atomic<bool> soundRunning(true);
    std::thread sound_thread;

    GameConfig config = runMenu();

    bool michaelMode = config.michaelMode;
    bool despawnApple = config.despawnApple;

    const char* selected_library = config.library.c_str();

    const char* available[] = {"sdl3", "sfml", "gl"};
    bool valid = false;

    for (auto & i : available)
        if (strcmp(selected_library, i) == 0)
            valid = true;

    if (!valid) {
        std::cerr << "Invalid library\n";
        return 1;
    }

    currentLibrary =
        (selected_library[0] == 's' && selected_library[1] == 'f') ? SFML :
        (selected_library[0] == 's' && selected_library[1] == 'd') ? SDL3 : GL;

    int width = config.width;
    int height = config.height;

    if (width <= 9 || height <= 9 || width > 100 || height > 100) {
        std::cerr << "Invalid size\n";
        return 1;
    }

    void* gui = nullptr;
    void* handle = nullptr;

    create_t create_gui = nullptr;
    destroy_t destroy_gui = nullptr;
    display_t display_gui = nullptr;
    input_t input_gui = nullptr;

    auto lib_path = [](int mode) {
        if (mode == SFML) return "./lib_sfml.so";
        if (mode == SDL3) return "./lib_sdl3.so";
        return "./lib_gl.so";
    };

    auto load_lib = [&](const char* lib, int mode) -> bool {

        if (handle) {
            destroy_gui(gui);
            dlclose(handle);
        }

        handle = dlopen(lib, RTLD_LAZY);
        if (!handle) return false;

        const char* suffix =
            (mode == SFML) ? "sfml" :
            (mode == SDL3) ? "sdl3" : "gl";

        create_gui = (create_t)dlsym(handle, (std::string("create_gui_") + suffix).c_str());
        destroy_gui = (destroy_t)dlsym(handle, (std::string("destroy_gui_") + suffix).c_str());
        display_gui = (display_t)dlsym(handle, (std::string("display_gui_") + suffix).c_str());
        input_gui = (input_t)dlsym(handle, (std::string("input_gui_") + suffix).c_str());

        if (!create_gui || !destroy_gui || !display_gui || !input_gui)
            return false;

        gui = create_gui(width, height, michaelMode);
        return true;
    };

    if (!load_lib(lib_path(currentLibrary), currentLibrary))
        return 1;

    Game game(width, height, michaelMode, despawnApple);

    bool running = true;
    auto last_move = std::chrono::high_resolution_clock::now();

    while (running) {

        display_gui(gui, game);

        int input = input_gui(gui);

        if (input == -1) running = false;

        else if (input == 10 && !load_lib(lib_path(SDL3), SDL3)) running = false;
        else if (input == 20 && !load_lib(lib_path(SFML), SFML)) running = false;
        else if (!michaelMode && input == 30 && !load_lib(lib_path(GL), GL)) running = false;

        else if (input == UP) game.changeDirection(UP);
        else if (input == DOWN) game.changeDirection(DOWN);
        else if (input == LEFT) game.changeDirection(LEFT);
        else if (input == RIGHT) game.changeDirection(RIGHT);

        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - last_move).count();

        if (elapsed >= TICK_RATE) {
            if (game.moveSnake(onAppleSound, onAppleMutex) == -1)
                running = false;

            last_move = now;
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    soundRunning = false;

    if (sound_thread.joinable())
        sound_thread.join();

    if (gui) destroy_gui(gui);
    if (handle) dlclose(handle);

    return 0;
}