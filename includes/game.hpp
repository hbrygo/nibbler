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

enum InputAction
{
    P2_UP = 1001,
    P2_DOWN = 1002,
    P2_LEFT = 1003,
    P2_RIGHT = 1004
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
        int _snakeSize2;
        int _nbPlayer;
        std::vector<std::pair<int, int>> _snakeBody;
        std::vector<std::pair<int, int>> _snakeBody2;
        Direction _currentDirection;
        Direction _currentDirection2;
        std::pair<int, int> _applePosition;
        std::vector<std::pair<int, int>> _wallPositions;
        struct timeval _spawnApple;
        bool _despawnApple;
        int _score;
        bool _michaelMode;
        bool _wallMode;

    public:
        Game();
        Game(int height, int width, int nbPlayer, bool michaelMode, bool despawnApple, bool wallMode);
        Game(const Game& other);
        Game& operator=(const Game& other);
        ~Game();

        int getWidth() const;
        int getHeight() const;
        int getNbPlayer() const;
        void setMichaelMode(bool enabled);
        bool getMichaelMode() const;
        void displayGameArea();
        void changeDirection(Direction direction);
        void changeDirection2(Direction direction);
        int moveSnake(int& onAppleSound, std::mutex& onAppleMutex);
        int moveSnake2(int& onAppleSound, std::mutex& onAppleMutex);
        int checkDeath();
        int checkDeath2();
        int onApple();
        int onApple2();
        void generateApple();
        timeval getSpawnApple() const;
        bool getAppelDespawned() const;
        int getCell(int x, int y) const;
        int getCurrentDirection() const;
        int getCurrentDirection2() const;
        std::vector<std::pair<int, int>>& getSnakeBody();
        const std::vector<std::pair<int, int>>& getSnakeBody() const;
        std::vector<std::pair<int, int>>& getSnakeBody2();
        const std::vector<std::pair<int, int>>& getSnakeBody2() const;
        bool getWallMode() const;
};

#endif