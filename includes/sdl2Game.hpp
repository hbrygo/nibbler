#ifndef SDL2GAME_HPP
# define SDL2GAME_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "nibbler.hpp"

class SDL2Game {
    private:
        SDL_Window* _window;
        SDL_Renderer* _renderer;
        int _width;
        int _height;
        SDL_Texture* _snakeUpDownTexture;
        SDL_Texture* _snakeLeftRightTexture;
        SDL_Texture* _snakeTurnRightTexture;
        SDL_Texture* _snakeTurnLeftTexture;
        SDL_Texture* _foodTexture;
        SDL_Texture* _backgroundTexture;
        SDL_Texture* _wallTexture;

    
    public:
        SDL2Game(int w, int h);
        ~SDL2Game();
        
        // Fonctions requises par le contrat dynamique
        void display(const Game& game);
        int handleInput();
};

#endif