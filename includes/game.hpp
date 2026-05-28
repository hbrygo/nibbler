#ifndef GAME_HPP
# define GAME_HPP

# include <iostream>
# include <vector>
# include <algorithm>
# include <random>
# include <sys/time.h>
# include <mutex>
# include <../miniaudio/miniaudio.h>

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
    SDL3 = 1,
    SFML = 2,
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
        struct timeval _spawnApple;
        bool _despawnApple;
        int _score;
        bool _michaelMode;

    public:
        Game();
        Game(int height, int width, bool michaelMode, bool despawnApple);
        Game(const Game& other);
        Game& operator=(const Game& other);
        ~Game();

        int getWidth() const;
        int getHeight() const;
        void setMichaelMode(bool enabled);
        bool getMichaelMode() const;
        void displayGameArea();
        void changeDirection(Direction direction);
        int moveSnake(int& onAppleSound, std::mutex& onAppleMutex);
        int checkDeath();
        int onApple();
        void generateApple();
        timeval getSpawnApple() const;
        bool getAppelDespawned() const;
        int getCell(int x, int y) const;
        int getCurrentDirection() const;
        std::vector<std::pair<int, int>>& getSnakeBody();
        const std::vector<std::pair<int, int>>& getSnakeBody() const;
};

#endif