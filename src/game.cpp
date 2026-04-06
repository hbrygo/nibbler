#include "nibbler.hpp"

Game::Game() : _gameAreaHeight(10), _gameAreaWidth(10) {
    _gameArea.resize(_gameAreaHeight, std::vector<int>(_gameAreaWidth, 0));
    _snakeSize = 4;
    for (int i = 0; i < _snakeSize; ++i) {
        _snakeBody.push_back({_gameAreaHeight / 2, _gameAreaWidth / 2 - i});
    }
    _currentDirection = RIGHT;
    generateApple();
}

Game::Game(int height, int width) : _gameAreaHeight(height), _gameAreaWidth(width) {
    _gameArea.resize(_gameAreaHeight, std::vector<int>(_gameAreaWidth, 0));
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

int Game::changeDirection(Direction direction) {
    if (direction == _currentDirection) {
        std::cout << "Snake is already moving in that direction!" << std::endl;
        return 0;
    } else if ((direction == UP && _currentDirection == DOWN) ||
               (direction == DOWN && _currentDirection == UP) ||
               (direction == LEFT && _currentDirection == RIGHT) ||
               (direction == RIGHT && _currentDirection == LEFT)) {
        std::cout << "Cannot move in the opposite direction!" << std::endl;
        return 0;
    }
    _currentDirection = direction;
    return 0;
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
    _applePosition = {rand() % _gameAreaHeight, rand() % _gameAreaWidth};
    while (std::find(_snakeBody.begin(), _snakeBody.end(), _applePosition) != _snakeBody.end()) {
        _applePosition = {rand() % _gameAreaHeight, rand() % _gameAreaWidth};
    }
}

int Game::moveSnake() {
    if (_currentDirection == UP) {
        for (auto segment = _snakeBody.end() - 1; segment != _snakeBody.begin() - 1; --segment) {
            if (segment != _snakeBody.begin()) {
                *segment = *(segment - 1);
            } else {
                segment->first -= 1;
            }
        }
    } else if (_currentDirection == DOWN) {
        for (auto segment = _snakeBody.end() - 1; segment != _snakeBody.begin() - 1; --segment) {
            if (segment != _snakeBody.begin()) {
                *segment = *(segment - 1);
            } else {
                segment->first += 1;
            }
        }
    } else if (_currentDirection == LEFT) {
        for (auto segment = _snakeBody.end() - 1; segment != _snakeBody.begin() - 1; --segment) {
            if (segment != _snakeBody.begin()) {
                *segment = *(segment - 1);
            } else {
                segment->second -= 1;
            }
        }
    } else if (_currentDirection == RIGHT) {
        for (auto segment = _snakeBody.end() - 1; segment != _snakeBody.begin() - 1; --segment) {
            if (segment != _snakeBody.begin()) {
                *segment = *(segment - 1);
            } else {
                segment->second += 1;
            }
        }
    }
    if (onApple()) {
        generateApple();
    }
    if (checkDeath()) {
        std::cout << "Game Over!" << std::endl;
        return -1;
    }
    return 0;
}