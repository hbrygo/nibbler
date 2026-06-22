#include "nibbler.hpp"
#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <cctype>
#include <string>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <random>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>

int currentLibrary = SDL3;

typedef void* (*create_t)(int, int, bool);
typedef void (*destroy_t)(void*);
typedef void (*display_t)(void*, const Game&);
typedef int (*input_t)(void*);

class DynamicDisplay : public INibblerDisplay {
public:
    DynamicDisplay(void* instance, create_t createFn, destroy_t destroyFn, display_t displayFn, input_t inputFn,
        int width, int height, bool michaelMode)
        : _instance(instance), _createFn(createFn), _destroyFn(destroyFn), _displayFn(displayFn), _inputFn(inputFn),
          _width(width), _height(height), _michaelMode(michaelMode) {}

    ~DynamicDisplay() override {
        stop();
    }

    bool init(int width, int height, bool michaelMode) override {
        _width = width;
        _height = height;
        _michaelMode = michaelMode;
        return _instance != nullptr;
    }

    int getEvents() override {
        return _instance ? _inputFn(_instance) : -1;
    }

    void updateGameData(const Game& game) override {
        _game = game;
    }

    void refreshDisplay() override {
        if (_instance) {
            _displayFn(_instance, _game);
        }
    }

    void stop() override {
        if (_instance) {
            _destroyFn(_instance);
            _instance = nullptr;
        }
    }

private:
    void* _instance;
    create_t _createFn;
    destroy_t _destroyFn;
    display_t _displayFn;
    input_t _inputFn;
    int _width;
    int _height;
    bool _michaelMode;
    Game _game;
};

enum AppMode {
    MODE_SOLO,
    MODE_LOCAL,
};

struct GameConfig {
    int width = 10;
    int height = 10;

    std::string library = "sdl3";

    bool michaelMode = false;
    bool despawnApple = false;
    bool wallMode = false;

    bool multiplayer = false;
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


GameConfig runMenu(char **argv)
{
    GameConfig c;
    try {
        c.width = atoi(argv[1]);
        if (c.width < 13 || c.width > 50) throw std::invalid_argument("Width must be at least 13 and max 50");
        c.height = atoi(argv[2]);
        if (c.height < 13 || c.height > 50) throw std::invalid_argument("Height must be at least 13 and max 50");
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        c.width = -1;
        return c;
    }

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
        c.wallMode = askChoice("Wall Mode?", {"Yes", "No"}) == 1;
        c.width = atoi(argv[1]);
        if (c.width < 10) c.width = 10;
        c.height = atoi(argv[2]);
        if (c.height < 10) c.height = 10;

        int lib = askChoice("Library", {"sdl3", "sfml", "gl"});
        c.library = (lib == 1 ? "sdl3" : lib == 2 ? "sfml" : "gl");

        return c;
    }

    c.multiplayer = true;
    int lib = askChoice("Library", {"sdl3", "sfml", "gl"});
    c.library = (lib == 1 ? "sdl3" : lib == 2 ? "sfml" : "gl");

    return c;
}

int main(int argc, char **argv)
{
    int onAppleSound = 0;
    std::mutex onAppleMutex;
    std::atomic<bool> soundRunning(true);
    std::thread sound_thread;
    std::unique_ptr<INibblerDisplay> gui;
    void* handle = nullptr;
    
    try {
        if (argc != 3) {
            throw std::invalid_argument("Usage: " + std::string(argv[0]) + " <width> <height>");
        }


        GameConfig config = runMenu(argv);
        if (config.width <= 13)
            throw std::invalid_argument("Width must be at least 13");


    bool michaelMode = config.michaelMode;
    bool despawnApple = config.despawnApple;
    bool wallMode = config.wallMode;

        AppMode mode = (config.multiplayer) ? MODE_LOCAL : MODE_SOLO;

        const char* selected_library = config.library.c_str();

        const char* available[] = {"sdl3", "sfml", "gl"};
        bool valid = false;

        for (auto & i : available)
            if (strcmp(selected_library, i) == 0)
                valid = true;

        if (!valid) {
            throw std::invalid_argument("Invalid library");
        }

        currentLibrary =
            (selected_library[0] == 's' && selected_library[1] == 'f') ? SFML :
            (selected_library[0] == 's' && selected_library[1] == 'd') ? SDL3 : GL;

        int width = config.width;
        int height = config.height;
        int nbPlayer = (mode == MODE_SOLO) ? 1 : 2;

        create_t create_gui = nullptr;
        destroy_t destroy_gui = nullptr;
        display_t display_gui = nullptr;
        input_t input_gui = nullptr;

        auto lib_path = [](int mode) {
            if (mode == SFML) return "./lib_sfml.so";
            if (mode == SDL3) return "./lib_sdl3.so";
            return "./lib_gl.so";
        };

        auto load_lib = [&](const char* lib, int mode) -> void {

            if (handle) {
                if (gui) {
                    gui->stop();
                    gui.reset();
                }
                dlclose(handle);
                handle = nullptr;
            }

            handle = dlopen(lib, RTLD_LAZY | RTLD_NODELETE);
            if (!handle) {
                throw std::runtime_error("Failed to load library [" + std::string(lib) + "]" + std::string(dlerror()));
            }

            const char* suffix =
                (mode == SFML) ? "sfml" :
                (mode == SDL3) ? "sdl3" : "gl";

            create_gui = (create_t)dlsym(handle, (std::string("create_gui_") + suffix).c_str());
            destroy_gui = (destroy_t)dlsym(handle, (std::string("destroy_gui_") + suffix).c_str());
            display_gui = (display_t)dlsym(handle, (std::string("display_gui_") + suffix).c_str());
            input_gui = (input_t)dlsym(handle, (std::string("input_gui_") + suffix).c_str());

            if (!create_gui || !destroy_gui || !display_gui || !input_gui) {
                if (handle) {
                    dlclose(handle);
                    handle = nullptr;
                }
                throw std::runtime_error("Failed to load symbols from library [" + std::string(lib) + "]" + std::string(dlerror()));
            }

            void* instance = create_gui(width, height, michaelMode);
            if (!instance && handle) {
                dlclose(handle);
                handle = nullptr;
                throw std::runtime_error("Failed to create GUI instance from library [" + std::string(lib) + "]");
            }   

            gui = std::unique_ptr<INibblerDisplay>(new DynamicDisplay(instance, create_gui, destroy_gui, display_gui, input_gui, width, height, michaelMode));
            if (!gui->init(width, height, michaelMode)) {
                gui->stop();
                gui.reset();
                dlclose(handle);
                handle = nullptr;
                throw std::runtime_error("Failed to initialize GUI");
            }
        };

        load_lib(lib_path(currentLibrary), currentLibrary);

        sound_thread = std::thread([&]() {
            ma_engine engine;
            if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
                std::cerr << "Failed to initialize audio engine" << std::endl;
                return ;
            }

            while (soundRunning) {
                bool shouldPlay = false;
                {
                    std::lock_guard<std::mutex> lock(onAppleMutex);
                    if (onAppleSound > 0) {
                        --onAppleSound;
                        shouldPlay = true;
                    }
                }

                if (shouldPlay) {
                    std::cout << "Playing apple sound effect!" << std::endl;
                    if (ma_engine_play_sound(&engine, "mp3/Yoshi Sound Ba-Dum (mlem).mp3", NULL) != MA_SUCCESS) {
                        std::cerr << "Failed to play sound" << std::endl;
                    }
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }

            ma_engine_uninit(&engine);
        });


    Game game(width, height, nbPlayer, michaelMode, despawnApple, wallMode);

        bool running = true;
        auto last_move = std::chrono::high_resolution_clock::now();

        while (running) {
            gui->updateGameData(game);
            gui->refreshDisplay();
            int input = gui->getEvents();

            switch (input) {
                case 10:
                    std::cout << "Switching to SDL3..." << std::endl;
                    load_lib(lib_path(SDL3), SDL3);
                    break;
                case 20:
                    std::cout << "Switching to SFML..." << std::endl;
                    load_lib(lib_path(SFML), SFML);
                    break;
                case 30:
                    if (!michaelMode) {
                        std::cout << "Switching to GL..." << std::endl;
                        load_lib(lib_path(GL), GL);
                    }
                    break;
                case 1000:
                    std::cout << "Pausing game. Press P to resume." << std::endl;
                    while (true) {
                        int pauseInput = gui->getEvents();
                        if (pauseInput == 1000) {
                            std::cout << "Resuming game." << std::endl;
                            break;
                        }
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    break;
                case UP:
                    game.changeDirection(UP, 1);
                    break;
                case DOWN:
                    game.changeDirection(DOWN, 1);
                    break;
                case LEFT:
                    game.changeDirection(LEFT, 1);
                    break;
                case RIGHT:
                    game.changeDirection(RIGHT, 1);
                    break;
                default:
                    break;
            }
            if (mode == MODE_LOCAL) {
                switch (input) {
                    case P2_UP:
                        game.changeDirection(UP, 2);
                        break;
                    case P2_DOWN:
                        game.changeDirection(DOWN, 2);
                        break;
                    case P2_LEFT:
                        game.changeDirection(LEFT, 2);
                        break;
                    case P2_RIGHT:
                        game.changeDirection(RIGHT, 2);
                        break;
                    default:
                        break;
                }
            }
            if (input == -1) running = false;

            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_move).count();

            if (elapsed >= TICK_RATE) {
                if (game.moveSnake(onAppleSound, onAppleMutex) == -1)
                    running = false;
                if (running && game.getNbPlayer() == 2 && game.moveSnake2(onAppleSound, onAppleMutex) == -1)
                    running = false;

                last_move = now;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "this is an exception" << std::endl;
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    soundRunning = false;

    if (sound_thread.joinable())
        sound_thread.join();
    if (gui) {
        gui->stop();
        gui.reset();
    }
    if (handle) dlclose(handle);
    _exit(0);
}