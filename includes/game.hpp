#ifndef GAME_HPP
# define GAME_HPP

# include <iostream>
# include <vector>
# include <algorithm>
# include <random>
# include <cstdint>

#define TICK_RATE 150

enum Direction
{
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    QUIT
};

enum CellType
{
    EMPTY,
    SNAKE,
    SNAKE_2,
    FOOD,
    WALL
};

enum GraphicLibrary {
    SFML = 1,
    SDL3 = 2,
    GL = 3
};

extern int currentLibrary;

class Game {
    private:
        int _gameAreaHeight;
        int _gameAreaWidth;
        std::vector<std::vector<CellType>> _gameArea;
        int _snakeSize;
        int _snakeSize2;
        int _nbPlayer;
        std::vector<std::pair<int, int>> _snakeBody;
        std::vector<std::pair<int, int>> _snakeBody2;
        Direction _currentDirection;
        Direction _currentDirection2;
        std::pair<int, int> _applePosition;
        std::pair<int, int> _wallPosition;
        std::mt19937 _rng;

    public:
        Game();
        Game(int height, int width);
        Game(int height, int width, int nbPlayer);
        Game(const Game& other);
        Game& operator=(const Game& other);
        ~Game();

        void setSeed(std::uint32_t seed);
        void displayGameArea();
        void changeDirection(Direction direction);
        void changeDirection2(Direction direction);
        int moveSnake();
        int moveSnake2();
        int checkDeath();
        int checkDeath2();
        int onApple();
        int onApple2();
        void generateApple();
        int getCell(int x, int y) const;
        int getCurrentDirection() const;
        int getCurrentDirection2() const;
        int getNbPlayer() const;
        std::vector<std::pair<int, int>>& getSnakeBody();
        const std::vector<std::pair<int, int>>& getSnakeBody() const;
        std::vector<std::pair<int, int>>& getSnakeBody2();
        const std::vector<std::pair<int, int>>& getSnakeBody2() const;
};

#endif