#include "nibbler.hpp"
#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <chrono>
#include <thread>

typedef void* (*create_t)(int, int);
typedef void (*destroy_t)(void*);
typedef void (*display_t)(void*, const Game&);
typedef int (*input_t)(void*);

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <width> <height>" << std::endl;
        return 1;
    }

    int width = std::atoi(argv[1]);
    int height = std::atoi(argv[2]);

    if (width <= 0 || height <= 0 || width > 1000 || height > 1000) {
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

    auto load_lib = [&](const char* lib_name) {
        if (handle) {
            destroy_gui(gui);
            dlclose(handle);
        }
        handle = dlopen(lib_name, RTLD_LAZY);
        if (!handle) {
            std::cerr << "Error loading " << lib_name << ": " << dlerror() << std::endl;
            return false;
        }
        create_gui = (create_t)dlsym(handle, "create_gui");
        destroy_gui = (destroy_t)dlsym(handle, "destroy_gui");
        display_gui = (display_t)dlsym(handle, "display_gui");
        input_gui = (input_t)dlsym(handle, "input_gui");
        if (!create_gui || !destroy_gui || !display_gui || !input_gui) {
            std::cerr << "Error: Missing symbols in library" << std::endl;
            dlclose(handle);
            return false;
        }
        gui = create_gui(width, height);
        return true;
    };

    // Charger la première lib (SDL2 par défaut)
    if (!load_lib("./lib_sdl2.so")) {
        return 1;
    }

    Game game(width, height);
    bool running = true;
	auto last_move = std::chrono::high_resolution_clock::now();

    while (running) {
        display_gui(gui, game);
        
        int input = input_gui(gui);
        std::cout << "Input received: " << input << std::endl;
        if (input == -1) running = false; // ESC
        else if (input == 1 && !load_lib("./lib_sdl2.so")) running = false;
        // else if (input == 2 && !load_lib("./lib_sfml.so")) running = false;
        // else if (input == 3 && !load_lib("./lib_ncurses.so")) running = false;
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