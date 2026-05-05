#ifndef GAME_HPP
# define GAME_HPP

# include <iostream>
# include <vector>
# include <algorithm>
# include <random>

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
    FOOD,
    WALL
};

enum GraphicLibrary {
    MLX = 1,
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
        std::vector<std::pair<int, int>> _snakeBody;
        Direction _currentDirection;
        std::pair<int, int> _applePosition;
        std::pair<int, int> _wallPosition;

    public:
        Game();
        Game(int height, int width);
        Game(const Game& other);
        Game& operator=(const Game& other);
        ~Game();

        void displayGameArea();
        void changeDirection(Direction direction);
        int moveSnake();
        int checkDeath();
        int onApple();
        void generateApple();
        int getCell(int x, int y) const;
        int getCurrentDirection() const;
        std::vector<std::pair<int, int>>& getSnakeBody();
        const std::vector<std::pair<int, int>>& getSnakeBody() const;
};

#endif