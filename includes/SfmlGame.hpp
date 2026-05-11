#ifndef SFML_GAME_HPP
# define SFML_GAME_HPP

# include "game.hpp"
# include "nibbler.hpp"
# include <SFML/Graphics.hpp>
# include <SFML/Window.hpp>
# include <dlfcn.h>
# include <iostream>
# include <chrono>

class SFMLGame {
    private:
        sf::RenderWindow* _window;
        int _width, _height;
        sf::Texture* _snakeUpDownTexture;
        sf::Texture* _snakeLeftRightTexture;
        sf::Texture* _snakeTurnRightTexture;
        sf::Texture* _snakeTurnLeftTexture;
        sf::Texture* _foodTexture;
        sf::Texture* _backgroundTexture1;
        sf::Texture* _backgroundTexture2;
        sf::Texture* _wallTexture;
        sf::Texture* _snakeHeadTexture;
        sf::Texture* _snakeTailTexture;
        std::chrono::high_resolution_clock::time_point _lastMoveTime;
        std::vector<std::pair<int, int>> _prevSnakeBody;

    public:
        SFMLGame(int w, int h);
        ~SFMLGame();
        void display(const Game& game);
        void display_good_part(sf::FloatRect rect, int currentDirection, const std::vector<std::pair<int, int>>& snakeBody);
        int handleInput();
};

#endif