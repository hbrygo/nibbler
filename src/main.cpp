#include "nibbler.hpp"
#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <string>
#include <chrono>
#include <thread>
#include <vector>

int currentLibrary = SDL3;

typedef void* (*create_t)(int, int);
typedef void (*destroy_t)(void*);
typedef void (*display_t)(void*, const Game&);
typedef int (*input_t)(void*);

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <width> <height> <sfml/sdl3/gl>" << std::endl;
        return 1;
    }
    const char* available_libraries[] = {"sfml", "sdl3", "gl"};
    const char* selected_library = argv[3];
    for (int i = 0; i < 3; i++) {
        if (strcmp(selected_library, available_libraries[i]) == 0) {
            break;
        }
        if (i == 3) {
            std::cerr << "Error: Invalid library. Available options are: sfml, sdl3, gl" << std::endl;
            return 1;
        }
    }

    currentLibrary = (selected_library[0] == 's' && selected_library[1] == 'f') ? SFML : 
                     (selected_library[0] == 's' && selected_library[1] == 'd') ? SDL3 : GL;

    int width = std::atoi(argv[1]);
    int height = std::atoi(argv[2]);

    if (width <= 9 || height <= 9 || width > 100 || height > 100) {
        std::cerr << "Error: Invalid width or height" << std::endl;
        return 1;
    }

    // Charger la première librairie graphique
    void* gui = nullptr;
    void* handle = nullptr;
    create_t create_gui = nullptr;
    destroy_t destroy_gui = nullptr;
    display_t display_gui = nullptr;
    input_t input_gui = nullptr;

    auto load_lib = [&](const char* lib_so, int currentLibrary) -> bool {
        const char* expected_suffix = (currentLibrary == SFML) ? "sfml" : (currentLibrary == SDL3) ? "sdl3" : "gl";
        const std::string create_symbol = std::string("create_gui_") + expected_suffix;
        const std::string destroy_symbol = std::string("destroy_gui_") + expected_suffix;
        const std::string display_symbol = std::string("display_gui_") + expected_suffix;
        const std::string input_symbol = std::string("input_gui_") + expected_suffix;
        if (handle) {
            destroy_gui(gui);
            dlclose(handle);
            handle = nullptr;
            gui = nullptr;
        }
        handle = dlopen(lib_so, RTLD_LAZY);
        if (!handle) {
            std::cerr << "Error loading " << lib_so << ": " << dlerror() << std::endl;
            return false;
        }

        create_gui = reinterpret_cast<create_t>(dlsym(handle, create_symbol.c_str()));
        destroy_gui = reinterpret_cast<destroy_t>(dlsym(handle, destroy_symbol.c_str()));
        display_gui = reinterpret_cast<display_t>(dlsym(handle, display_symbol.c_str()));
        input_gui = reinterpret_cast<input_t>(dlsym(handle, input_symbol.c_str()));
        if (!create_gui || !destroy_gui || !display_gui || !input_gui) {
            std::cerr << "Error: Missing symbols in library" << std::endl;
            dlclose(handle);
            handle = nullptr;
            return false;
        }
        gui = create_gui(width, height);
        return true;
    };

    // Charger la première lib (SDL3 par défaut)
    if (!load_lib("./lib_sdl3.so", currentLibrary)) {
        return 1;
    }

    Game game(width, height);
    bool running = true;
	auto last_move = std::chrono::high_resolution_clock::now();

    while (running) {
        display_gui(gui, game);
        
        int input = input_gui(gui);
        if (input == -1) running = false; // ESC
        else if (input == 10 && !load_lib("./lib_sdl3.so", currentLibrary)) running = false;  // Mode 1 (was 1)
        // else if (input == 20 && !load_lib("./lib_other.so", currentLibrary)) running = false;  // Mode 2 (was 2)
        // else if (input == 30 && !load_lib("./lib_third.so", currentLibrary)) running = false;  // Mode 3 (was 3)
        else if (input == UP) game.changeDirection(UP);
        else if (input == DOWN) game.changeDirection(DOWN);
        else if (input == LEFT) game.changeDirection(LEFT);
        else if (input == RIGHT) game.changeDirection(RIGHT);

        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_move).count();
        
        if (elapsed >= TICK_RATE) {
            if (game.moveSnake() == -1) running = false; // Collision
            last_move = now;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    if (gui) destroy_gui(gui);
    if (handle) dlclose(handle);

    return 0;
}
