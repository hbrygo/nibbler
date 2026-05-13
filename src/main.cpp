#include "nibbler.hpp"
#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <string>
#include <chrono>
#include <thread>
#include <vector>

int currentLibrary = SDL3;

typedef void* (*create_t)(int, int, bool);
typedef void (*destroy_t)(void*);
typedef void (*display_t)(void*, const Game&);
typedef int (*input_t)(void*);

int main(int argc, char** argv) {
    bool michaelMode = false;
    if (argc == 2 && std::strcmp(argv[1], "projet_michael") == 0) michaelMode = true;
    else if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <width> <height> <sfml/sdl3/gl>" << std::endl;
        return 1;
    }
    const char* selected_library;
    if (michaelMode) {
        std::cout << "Michael Mode Activated!" << std::endl;
        selected_library = "sdl3";
    } else {
        selected_library = argv[3];
    }
    bool valid_library = false;
    const char* available_libraries[] = {"sdl3", "sfml", "gl"};
    for (int i = 0; i < 3; ++i) {
        if (strcmp(selected_library, available_libraries[i]) == 0) {
            valid_library = true;
            break;
        }
    }
    if (!valid_library) {
        std::cerr << "Error: Invalid library. Available options are: sfml, sdl3, gl" << std::endl;
        return 1;
    }

    currentLibrary = (selected_library[0] == 's' && selected_library[1] == 'f') ? SFML : 
                    (selected_library[0] == 's' && selected_library[1] == 'd') ? SDL3 : GL;
    int width = 0;
    int height = 0;
    if (michaelMode) {
        width = 17;
        height = 32;
    } else {
        width = std::atoi(argv[1]);
        height = std::atoi(argv[2]);
    }
    if (width <= 9 || height <= 9 || width > 100 || height > 100) {
        std::cerr << "Error: Invalid width or height" << std::endl;
        return 1;
    }

    void* gui = nullptr;
    void* handle = nullptr;
    create_t create_gui = nullptr;
    destroy_t destroy_gui = nullptr;
    display_t display_gui = nullptr;
    input_t input_gui = nullptr;

    auto lib_path_for_mode = [](int mode) -> const char* {
        if (mode == SFML) {
            return "./lib_sfml.so";
        }
        if (mode == SDL3) {
            return "./lib_sdl3.so";
        }
        return "./lib_gl.so";
    };

    auto load_lib = [&](const char* lib_so, int mode) -> bool {
        const char* expected_suffix = (mode == SFML) ? "sfml" : (mode == SDL3) ? "sdl3" : "gl";
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
        gui = create_gui(width, height, michaelMode);
        return true;
    };

    if (!load_lib(lib_path_for_mode(currentLibrary), currentLibrary)) {
        return 1;
    }

    Game game(width, height, michaelMode);
    bool running = true;
	auto last_move = std::chrono::high_resolution_clock::now();

    while (running) {
        display_gui(gui, game);
        
        int input = input_gui(gui);
        if (input == -1) running = false; // ESC
        else if (input == 10 && !load_lib(lib_path_for_mode(SDL3), SDL3)) running = false;
        else if (input == 20 && !load_lib(lib_path_for_mode(SFML), SFML)) running = false;
        else if (!michaelMode && input == 30 && !load_lib(lib_path_for_mode(GL), GL)) running = false;
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
