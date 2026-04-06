#ifndef GAME_HPP
# define GAME_HPP

# include <iostream>
# include <vector>
# include <algorithm>

enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE,
    QUIT
};

class Game {
    private:
        int _gameAreaHeight;
        int _gameAreaWidth;
        std::vector<std::vector<int>> _gameArea;
        int _snakeSize;
        std::vector<std::pair<int, int>> _snakeBody;
        Direction _currentDirection;
        std::pair<int, int> _applePosition;

    public:
        Game();
        Game(int height, int width);
        Game(const Game& other);
        Game& operator=(const Game& other);
        ~Game();

        void displayGameArea();
        int changeDirection(Direction direction);
        int moveSnake();
        int checkDeath();
        int onApple();
        void generateApple();
};

#endif