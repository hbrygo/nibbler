#ifndef SDL3GAME_HPP
# define SDL3GAME_HPP

# include <iostream>
# include <dlfcn.h>
# include <cstdlib>
# include <map>
# include "game.hpp"
# include "nibbler.hpp"

// SDL3 opaque types
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
typedef struct SDL_Surface SDL_Surface;

// SDL3 keyboard event structure (from SDL_events.h)
struct SDL_KeyboardEvent {
    unsigned int type;         // SDL_EventType (4 bytes) - offset 0
    unsigned int reserved;     // (4 bytes) - offset 4
    unsigned long timestamp;   // Uint64 - nanoseconds (8 bytes) - offset 8
    unsigned int windowID;     // SDL_WindowID (4 bytes) - offset 16
    unsigned int which;        // SDL_KeyboardID (4 bytes) - offset 20
    int scancode;              // SDL_Scancode (4 bytes) - offset 24 ← THIS IS WHAT WE WANT
    int keycode;               // SDL_Keycode (4 bytes) - offset 28
    unsigned int mod;          // SDL_Keymod (4 bytes) - offset 32
    unsigned short raw;        // platform scancode (2 bytes) - offset 36
    unsigned char down;        // bool (1 byte) - offset 38
    unsigned char repeat;      // bool (1 byte) - offset 39
};

// SDL3 event structure with union (correct from SDL3 headers)
struct SDL_Event {
    unsigned int type;
    union {
        SDL_KeyboardEvent key;
        char padding[128];
    };
};

struct SDL_Rect {
    int x, y, w, h;
};

struct SDL_FPoint {
    float x, y;
};

typedef int SDL_FlipMode;

struct SDL_FRect {
    float x, y, w, h;
};

class SDL3Game {
    private:
        SDL_Window* _window;
        SDL_Renderer* _renderer;
        int _width, _height;
        SDL_Texture* _snakeUpDownTexture;
        SDL_Texture* _snakeUpDownTexture2;
        SDL_Texture* _snakeLeftRightTexture;
        SDL_Texture* _snakeLeftRightTexture2;
        SDL_Texture* _snakeTurnRightTexture;
        SDL_Texture* _snakeTurnRightTexture2;
        SDL_Texture* _snakeTurnLeftTexture;
        SDL_Texture* _snakeTurnLeftTexture2;
        SDL_Texture* _foodTexture;
        SDL_Texture* _backgroundTexture;
        SDL_Texture* _wallTexture;
        SDL_Texture* _michaelModeTexture;

    public:
        SDL3Game();
        SDL3Game(int w, int h, bool michaelMode);
        ~SDL3Game();
        SDL3Game(const SDL3Game&);
        SDL3Game& operator=(const SDL3Game&);
        void display(const Game& game);
        void display_good_part(SDL_FRect rect, int currentDirection, const std::vector<std::pair<int, int>>& _snakeBody, bool modeMichael);
        void display_good_part2(SDL_FRect rect, int currentDirection, const std::vector<std::pair<int, int>>& _snakeBody, bool modeMichael);
        int handleInput();
};

#endif