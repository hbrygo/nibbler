#ifndef SFMLGAME_HPP
# define SFMLGAME_HPP

#include "game.hpp"
#include "nibbler.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <dlfcn.h>
#include <iostream>
#include <chrono>

class SFMLGame {
    private:
        sf::RenderWindow* _window;
        int _width, _height;
        bool _michaelMode;
        sf::Texture* _snakeUpDownTexture;
        sf::Texture* _snake2UpDownTexture;
        sf::Texture* _snakeLeftRightTexture;
        sf::Texture* _snake2LeftRightTexture;
        sf::Texture* _snakeTurnRightTexture;
        sf::Texture* _snake2TurnRightTexture;
        sf::Texture* _snakeTurnLeftTexture;
        sf::Texture* _snake2TurnLeftTexture;
        sf::Texture* _foodTexture;
        sf::Texture* _backgroundTexture1;
        sf::Texture* _backgroundTexture2;
        sf::Texture* _wallTexture;
        sf::Texture* _snakeHeadTexture;
        sf::Texture* _snake2HeadTexture;
        sf::Texture* _snakeHeadTurnLeftTexture;
        sf::Texture* _snake2HeadTurnLeftTexture;
        sf::Texture* _snakeHeadTurnRightTexture;
        sf::Texture* _snake2HeadTurnRightTexture;
        sf::Texture* _snakeTailTexture;
        sf::Texture* _snake2TailTexture;
        sf::Texture* _michaelModeTexture;
        std::chrono::high_resolution_clock::time_point _lastMoveTime;
        std::chrono::high_resolution_clock::time_point _lastMoveTime2;
        std::vector<std::pair<int, int>> _prevSnakeBody;
        std::vector<std::pair<int, int>> _prevSnakeBody2;

    public:
        SFMLGame();
        SFMLGame(int w, int h, bool michaelMode);
        ~SFMLGame();
        SFMLGame(const SFMLGame&);
        SFMLGame& operator=(const SFMLGame&);
        void display(const Game& game);
        void display_good_part(int currentDirection, const std::vector<std::pair<int, int>>& snakeBody, bool modeMichael,
            std::chrono::high_resolution_clock::time_point& lastMoveTime,
            std::vector<std::pair<int, int>>& prevSnakeBody);
        void display_good_part2(int currentDirection, const std::vector<std::pair<int, int>>& snakeBody, bool modeMichael,
            std::chrono::high_resolution_clock::time_point& lastMoveTime,
            std::vector<std::pair<int, int>>& prevSnakeBody);
        int handleInput();
};

#endif