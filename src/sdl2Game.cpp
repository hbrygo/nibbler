#include "sdl2Game.hpp"
#include <iostream>

SDL2Game::SDL2Game(int w, int h) : _window(nullptr), _renderer(nullptr), _width(w), _height(h) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return;
    }

    _window = SDL_CreateWindow("Nibbler - SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                w * 32, h * 32, SDL_WINDOW_SHOWN);
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!_window || !_renderer) {
        std::cerr << "SDL_CreateWindow/Renderer failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
    }

    _snakeUpDownTexture = IMG_LoadTexture(_renderer, "textureSDL2/snake_up_down.png");
    _snakeLeftRightTexture = IMG_LoadTexture(_renderer, "textureSDL2/snake_right_left.png");
    _snakeTurnRightTexture = IMG_LoadTexture(_renderer, "textureSDL2/snake_turn_right.png");
    _snakeTurnLeftTexture = IMG_LoadTexture(_renderer, "textureSDL2/snake_turn_left.png");
    _foodTexture = IMG_LoadTexture(_renderer, "textureSDL2/apple.png");
    _backgroundTexture = IMG_LoadTexture(_renderer, "textureSDL2/bg.png");
    _wallTexture = IMG_LoadTexture(_renderer, "textureSDL2/wall.png");

    if (!_snakeUpDownTexture || !_snakeLeftRightTexture || !_snakeTurnRightTexture || !_snakeTurnLeftTexture || !_foodTexture || !_backgroundTexture || !_wallTexture) {
        std::cerr << "Failed to load textures: " << IMG_GetError() << std::endl;
        SDL_Quit();
    }
}

SDL2Game::~SDL2Game() {
    if (_snakeUpDownTexture) SDL_DestroyTexture(_snakeUpDownTexture);
    if (_snakeLeftRightTexture) SDL_DestroyTexture(_snakeLeftRightTexture);
    if (_snakeTurnRightTexture) SDL_DestroyTexture(_snakeTurnRightTexture);
    if (_snakeTurnLeftTexture) SDL_DestroyTexture(_snakeTurnLeftTexture);
    if (_foodTexture) SDL_DestroyTexture(_foodTexture);
    if (_backgroundTexture) SDL_DestroyTexture(_backgroundTexture);
    if (_wallTexture) SDL_DestroyTexture(_wallTexture);
    if (_renderer) SDL_DestroyRenderer(_renderer);
    if (_window) SDL_DestroyWindow(_window);
    SDL_Quit();
}

static void getSnakeSegmentDirections(const Game& game, int x, int y, int& dirA, int& dirB)
{
    dirA = dirB = NONE;

    if (game.getCell(x, y - 1) == SNAKE) {
        if (dirA == NONE) dirA = UP;
        else dirB = UP;
    }
    if (game.getCell(x, y + 1) == SNAKE) {
        if (dirA == NONE) dirA = DOWN;
        else dirB = DOWN;
    }
    if (game.getCell(x - 1, y) == SNAKE) {
        if (dirA == NONE) dirA = LEFT;
        else dirB = LEFT;
    }
    if (game.getCell(x + 1, y) == SNAKE) {
        if (dirA == NONE) dirA = RIGHT;
        else dirB = RIGHT;
    }
}

void SDL2Game::display(const Game& game) {
    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
    SDL_RenderClear(_renderer);

    for (int y = 0; y < _height; ++y) {
        for (int x = 0; x < _width; ++x) {
            SDL_Rect rect = { x * 32, y * 32, 32, 32 };
            SDL_RenderCopy(_renderer, _backgroundTexture, nullptr, &rect);

            int cell = game.getCell(x, y);
            if (cell == FOOD) {
                std::cout << "Rendering food at (" << x << ", " << y << ")" << std::endl;
                SDL_RenderCopy(_renderer, _foodTexture, nullptr, &rect);
            } else if (cell == SNAKE) {
                int dirA = NONE;
                int dirB = NONE;
                getSnakeSegmentDirections(game, x, y, dirA, dirB);

                if (dirB == NONE) {
                    if (dirA == LEFT || dirA == RIGHT) {
                        SDL_RenderCopy(_renderer, _snakeLeftRightTexture, nullptr, &rect);
                    } else {
                        SDL_RenderCopy(_renderer, _snakeUpDownTexture, nullptr, &rect);
                    }
                } else if ((dirA == UP && dirB == DOWN) || (dirA == DOWN && dirB == UP)) { // Vertical segment
                    SDL_RenderCopy(_renderer, _snakeUpDownTexture, nullptr, &rect);
                } else if ((dirA == LEFT && dirB == RIGHT) || (dirA == RIGHT && dirB == LEFT)) { // Horizontal segment
                    SDL_RenderCopy(_renderer, _snakeLeftRightTexture, nullptr, &rect);
                } else if ((dirA == DOWN && dirB == RIGHT)) { // Turn from down to right
                    SDL_RenderCopyEx(_renderer, _snakeTurnLeftTexture, nullptr, &rect, 270, nullptr, SDL_FLIP_VERTICAL);
                } else if ((dirA == UP && dirB == LEFT) || (dirA == LEFT && dirB == UP)) { // Turn from up to left
                    SDL_RenderCopyEx(_renderer, _snakeTurnLeftTexture, nullptr, &rect, 0, nullptr, SDL_FLIP_NONE);
                } else if ((dirA == DOWN && dirB == LEFT) || (dirA == LEFT && dirB == DOWN)) { // Turn from down to left
                    // std::cout << "in Unexpected snake segment at (" << x << ", " << y << ") with directions " << dirA << " and " << dirB << std::endl;
                    SDL_RenderCopyEx(_renderer, _snakeTurnRightTexture, nullptr, &rect, 270, nullptr, SDL_FLIP_VERTICAL);
                } else if ((dirA == UP && dirB == RIGHT) || (dirA == RIGHT && dirB == UP)) { // Turn from up to right
                    SDL_RenderCopyEx(_renderer, _snakeTurnRightTexture, nullptr, &rect, 180, nullptr, SDL_FLIP_NONE);
                }
                // std::cout << "Unexpected snake segment at (" << (dirA == LEFT ? "left" : dirA == RIGHT ? "right" : "default") << ", " << (dirA == UP ? "up" : dirA == DOWN ? "down" : "default") << ") with directions " << dirA << " and " << dirB << std::endl;
            }
        }
    }

    SDL_RenderPresent(_renderer);
}

int SDL2Game::handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) return -1;
        if (event.type == SDL_KEYDOWN) {
            std::cout << "Key pressed: " << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: return -1;
                case SDLK_UP: return UP;
                case SDLK_DOWN: return DOWN;
                case SDLK_LEFT: return LEFT;
                case SDLK_RIGHT: return RIGHT;
                case SDLK_1: return 1;
                case SDLK_2: return 2;
                case SDLK_3: return 3;
                default: break;
            }
        }
    }
    return 0;
}

extern "C" {
    void* create_gui(int width, int height) {
        return new SDL2Game(width, height);
    }
    
    void destroy_gui(void* gui) {
        delete (SDL2Game*)gui;
    }
    
    void display_gui(void* gui, const Game& game) {
        ((SDL2Game*)gui)->display(game);
    }
    
    int input_gui(void* gui) {
        return ((SDL2Game*)gui)->handleInput();
    }
}