#include "nibbler.hpp"

Game::Game() : _gameAreaHeight(10), _gameAreaWidth(10), _wallPosition({-1, -1}) {
    _gameArea.resize(_gameAreaHeight, std::vector<CellType>(_gameAreaWidth, EMPTY));
    _snakeSize = 4;
    for (int i = 0; i < _snakeSize; ++i) {
        _snakeBody.push_back({_gameAreaHeight / 2, _gameAreaWidth / 2 - i});
    }
    _currentDirection = RIGHT;
    generateApple();
}

Game::Game(int height, int width, bool michaelMode) : _gameAreaHeight(height), _gameAreaWidth(width), _wallPosition({-1, -1}), _score(0), _michaelMode(michaelMode) {
    _gameArea.resize(_gameAreaHeight, std::vector<CellType>(_gameAreaWidth, EMPTY));
    _snakeSize = 4;
    for (int i = 0; i < _snakeSize; ++i) {
        _snakeBody.push_back({_gameAreaHeight / 2, _gameAreaWidth / 2 - i});
    }
    _currentDirection = RIGHT;
    generateApple();
}

Game::Game(const Game& other) : _gameAreaHeight(other._gameAreaHeight), _gameAreaWidth(other._gameAreaWidth), _gameArea(other._gameArea), _snakeSize(other._snakeSize), _snakeBody(other._snakeBody), _applePosition(other._applePosition) {}

Game& Game::operator=(const Game& other) {
    if (this != &other) {
        _gameAreaHeight = other._gameAreaHeight;
        _gameAreaWidth = other._gameAreaWidth;
        _gameArea = other._gameArea;
        _snakeSize = other._snakeSize;
        _snakeBody = other._snakeBody;
        _applePosition = other._applePosition;
    }
    return *this;
}

Game::~Game() {}

void Game::displayGameArea()
{
    for (int i = 0; i < _gameAreaHeight; ++i) {
        for (int j = 0; j < _gameAreaWidth; ++j) {
            bool printed = false;
            for (const auto &segment : _snakeBody) {
                if (segment.first == i && segment.second == j) {
                    std::cout << "S ";
                    printed = true;
                    break;
                }
            }
            if (!printed) {
                if (_applePosition.first == i && _applePosition.second == j)
                    std::cout << "A ";
                else
                    std::cout << "0 ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Game::changeDirection(Direction direction) {
    // Vérifier qu'on ne fait pas demi-tour
    if ((getCurrentDirection() == UP && direction == DOWN) ||
        (getCurrentDirection() == DOWN && direction == UP) ||
        (getCurrentDirection() == LEFT && direction == RIGHT) ||
        (getCurrentDirection() == RIGHT && direction == LEFT)) {
        std::cerr << "Snake is already moving in that direction!" << std::endl;
        return;
    }
    std::cout << "Changing direction to: " << (direction == UP ? "UP" : direction == DOWN ? "DOWN" : direction == LEFT ? "LEFT" : "RIGHT") << std::endl;
    _currentDirection = direction;
}

int Game::checkDeath() {
    const auto& head = _snakeBody.front();
    // wall
    if (head.first < 0 || head.first >= _gameAreaHeight || head.second < 0 || head.second >= _gameAreaWidth) {
        return -1;
    }
    // snake
    for (size_t i = 1; i < _snakeBody.size(); ++i) {
        if (head == _snakeBody[i]) {
            return -1;
        }
    }
    return 0;
}

int Game::onApple() {
    const auto& head = _snakeBody.front();
    if (head == _applePosition) {
        _snakeSize++;
        _snakeBody.push_back(_snakeBody.back());
        return 1;
    }
    return 0;
}

void Game::generateApple()
{
    if (_snakeBody.size() >= static_cast<size_t>(_gameAreaHeight) * static_cast<size_t>(_gameAreaWidth)) {
        _applePosition = {-1, -1};
        return;
    }

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> rowDist(0, _gameAreaHeight - 1);
    std::uniform_int_distribution<int> colDist(0, _gameAreaWidth - 1);

    std::pair<int,int> pos;
    do {
        pos = { rowDist(rng), colDist(rng) };
    } while (std::find(_snakeBody.begin(), _snakeBody.end(), pos) != _snakeBody.end());

    _applePosition = pos;
    gettimeofday(&_spawnApple, nullptr);
}

int Game::moveSnake(int& onAppleSound, std::mutex& onAppleMutex) {
    if (_snakeBody.empty()) {
        return -1;
    }

    std::pair<int, int> newHead = _snakeBody.front();

    if (_currentDirection == UP) {
        newHead.second -= 1;
    } else if (_currentDirection == DOWN) {
        newHead.second += 1;
    } else if (_currentDirection == LEFT) {
        newHead.first -= 1;
    } else if (_currentDirection == RIGHT) {
        newHead.first += 1;
    } else {
        return 0;
    }

    for (size_t i = _snakeBody.size() - 1; i > 0; --i) {
        _snakeBody[i] = _snakeBody[i - 1];
    }
    _snakeBody[0] = newHead;

    if (onApple()) {
        struct timeval now;
        gettimeofday(&now, nullptr);
        long elapsed = (now.tv_sec - _spawnApple.tv_sec) * 2;
        if (elapsed > 100) elapsed = 100;
        _score += 100 - elapsed;
        std::cout << "This apple give you " << 100 - elapsed << " points!" << std::endl;
        {
            std::lock_guard<std::mutex> lock(onAppleMutex);
            onAppleSound++;
        }
        generateApple();
    }

    if (checkDeath()) {
        std::cout << "Score: " << _score << std::endl;
        std::cout << "Game Over!" << std::endl;
        return -1;
    }

    return 0;
}

int Game::getCell(int x, int y) const {
    for (const auto& segment : _snakeBody) {
        if (segment.first == x && segment.second == y) {
            return SNAKE;
        }
    }
    if (_applePosition.first == x && _applePosition.second == y) {
        return FOOD;
    }
    if ((_wallPosition.first == x && _wallPosition.second == y) || (x < 0 || x >= _gameAreaHeight) || (y < 0 || y >= _gameAreaWidth)) {
        return WALL;
    }
    return EMPTY;
}

int Game::getCurrentDirection() const {
    return _currentDirection;
}

const std::vector<std::pair<int, int>>& Game::getSnakeBody() const {
    return _snakeBody;
}

void Game::setMichaelMode(bool enabled) {
    _michaelMode = enabled;
}

bool Game::getMichaelMode() const {
    return _michaelMode;
}