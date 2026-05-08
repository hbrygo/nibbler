#include "nibbler.hpp"

Game::Game() : _gameAreaHeight(10), _gameAreaWidth(10), _nbPlayer(1), _wallPosition({-1, -1}), _rng(std::random_device{}()) {
    _gameArea.resize(_gameAreaHeight, std::vector<CellType>(_gameAreaWidth, EMPTY));
    _snakeSize = 4;
    for (int i = 0; i < _snakeSize; ++i) {
        _snakeBody.push_back({_gameAreaHeight / 2, _gameAreaWidth / 2 - i});
    }
    _currentDirection = RIGHT;
    generateApple();
}

Game::Game(int height, int width) : _gameAreaHeight(height), _gameAreaWidth(width), _nbPlayer(1), _wallPosition({-1, -1}), _rng(std::random_device{}()) {
    _gameArea.resize(_gameAreaHeight, std::vector<CellType>(_gameAreaWidth, EMPTY));
    _snakeSize = 4;
    for (int i = 0; i < _snakeSize; ++i) {
        _snakeBody.push_back({_gameAreaHeight / 2, _gameAreaWidth / 2 - i});
    }
    _currentDirection = RIGHT;
    generateApple();
}

Game::Game(int height, int width, int nbPlayer) : _gameAreaHeight(height), _gameAreaWidth(width), _nbPlayer(nbPlayer), _wallPosition({-1, -1}), _rng(std::random_device{}()) {
    _gameArea.resize(_gameAreaHeight, std::vector<CellType>(_gameAreaWidth, EMPTY));
    _snakeSize = 4;
    _snakeSize2 = 4;

    int leftHeadCol = std::max(3, _gameAreaWidth / 4 + 1);
    int rightHeadCol = std::max(_gameAreaWidth - 4, 4);

    for (int i = 0; i < _snakeSize; ++i) {
        _snakeBody.push_back({_gameAreaHeight / 2, leftHeadCol - i});
        _snakeBody2.push_back({_gameAreaHeight / 2 + 1, rightHeadCol + i});
    }
    _currentDirection = RIGHT;
    _currentDirection2 = LEFT;
    generateApple();
}

Game::Game(const Game& other) : _gameAreaHeight(other._gameAreaHeight), _gameAreaWidth(other._gameAreaWidth), _gameArea(other._gameArea), _snakeSize(other._snakeSize), _snakeSize2(other._snakeSize2), _nbPlayer(other._nbPlayer), _snakeBody(other._snakeBody), _snakeBody2(other._snakeBody2), _currentDirection(other._currentDirection), _currentDirection2(other._currentDirection2), _applePosition(other._applePosition), _wallPosition(other._wallPosition), _rng(other._rng) {}

Game& Game::operator=(const Game& other) {
    if (this != &other) {
        _gameAreaHeight = other._gameAreaHeight;
        _gameAreaWidth = other._gameAreaWidth;
        _gameArea = other._gameArea;
        _snakeSize = other._snakeSize;
        _snakeBody = other._snakeBody;
        _applePosition = other._applePosition;
        _snakeSize2 = other._snakeSize2;
        _snakeBody2 = other._snakeBody2;
        _currentDirection = other._currentDirection;
        _currentDirection2 = other._currentDirection2;
        _wallPosition = other._wallPosition;
        _nbPlayer = other._nbPlayer;
        _rng = other._rng;
    }
    return *this;
}

Game::~Game() {}

void Game::setSeed(std::uint32_t seed) {
    _rng.seed(seed);
    generateApple();
}

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

void Game::changeDirection2(Direction direction) {
    // Vérifier qu'on ne fait pas demi-tour
    if ((getCurrentDirection2() == UP && direction == DOWN) ||
        (getCurrentDirection2() == DOWN && direction == UP) ||
        (getCurrentDirection2() == LEFT && direction == RIGHT) ||
        (getCurrentDirection2() == RIGHT && direction == LEFT)) {
        std::cerr << "Snake 2 is already moving in that direction!" << std::endl;
        return;
    }
    std::cout << "Changing direction for Snake 2 to: " << (direction == UP ? "UP" : direction == DOWN ? "DOWN" : direction == LEFT ? "LEFT" : "RIGHT") << std::endl;
    _currentDirection2 = direction;
}

int Game::checkDeath() {
    const auto& head = _snakeBody.front();
    // wall
    if (head.first < 0 || head.first >= _gameAreaHeight || head.second < 0 || head.second >= _gameAreaWidth) {
        return -1;
    }
    // snake self collision
    for (size_t i = 1; i < _snakeBody.size(); ++i) {
        if (head == _snakeBody[i]) {
            return -1;
        }
    }
    // snake 2 collision
    if (_nbPlayer >= 2) {
        for (const auto& segment : _snakeBody2) {
            if (head == segment) {
                return -1;
            }
        }
    }
    return 0;
}

int Game::checkDeath2() {
    if (_nbPlayer < 2) return 0;

    const auto& head = _snakeBody2.front();
    // wall
    if (head.first < 0 || head.first >= _gameAreaHeight || head.second < 0 || head.second >= _gameAreaWidth) {
        return -1;
    }
    // snake self collision
    for (size_t i = 1; i < _snakeBody2.size(); ++i) {
        if (head == _snakeBody2[i]) {
            return -1;
        }
    }
    // collision with snake 1
    for (const auto& segment : _snakeBody) {
        if (head == segment) {
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

int Game::onApple2() {
    if (_nbPlayer < 2) return 0;
    const auto& head = _snakeBody2.front();
    if (head == _applePosition) {
        _snakeSize2++;
        _snakeBody2.push_back(_snakeBody2.back());
        return 1;
    }
    return 0;
}

void Game::generateApple()
{
    size_t occupiedCells = _snakeBody.size();
    if (_nbPlayer >= 2) {
        occupiedCells += _snakeBody2.size();
    }

    if (occupiedCells >= static_cast<size_t>(_gameAreaHeight) * static_cast<size_t>(_gameAreaWidth)) {
        _applePosition = {-1, -1};
        return;
    }

    std::uniform_int_distribution<int> rowDist(0, _gameAreaHeight - 1);
    std::uniform_int_distribution<int> colDist(0, _gameAreaWidth - 1);

    std::pair<int,int> pos;
    do {
        pos = { rowDist(_rng), colDist(_rng) };
    } while (
        std::find(_snakeBody.begin(), _snakeBody.end(), pos) != _snakeBody.end() ||
        (_nbPlayer >= 2 && std::find(_snakeBody2.begin(), _snakeBody2.end(), pos) != _snakeBody2.end())
    );

    _applePosition = pos;
}

int Game::moveSnake() {
    if (_snakeBody.empty()) {
        return -1;
    }

    std::pair<int, int> newHead = _snakeBody.front();

    if (_currentDirection == UP) {
        newHead.first -= 1;
    } else if (_currentDirection == DOWN) {
        newHead.first += 1;
    } else if (_currentDirection == LEFT) {
        newHead.second -= 1;
    } else if (_currentDirection == RIGHT) {
        newHead.second += 1;
    } else {
        return 0;
    }

    for (size_t i = _snakeBody.size() - 1; i > 0; --i) {
        _snakeBody[i] = _snakeBody[i - 1];
    }
    _snakeBody[0] = newHead;

    if (onApple()) {
        generateApple();
    }

    if (checkDeath()) {
        std::cout << "Game Over!" << std::endl;
        return -1;
    }

    return 0;
}

int Game::moveSnake2() {
    std::cout << _nbPlayer << " players in the game." << std::endl;
    if (_nbPlayer != 2) {
        std::cout << "Only one player mode, Snake 2 cannot move!" << std::endl;
        return 0;
    }

    if (_snakeBody2.empty()) {
        return -1;
    }

    std::pair<int, int> newHead = _snakeBody2.front();

    if (_currentDirection2 == UP) {
        newHead.first -= 1;
    } else if (_currentDirection2 == DOWN) {
        newHead.first += 1;
    } else if (_currentDirection2 == LEFT) {
        newHead.second -= 1;
    } else if (_currentDirection2 == RIGHT) {
        newHead.second += 1;
    } else {
        return 0;
    }

    for (size_t i = _snakeBody2.size() - 1; i > 0; --i) {
        _snakeBody2[i] = _snakeBody2[i - 1];
    }
    _snakeBody2[0] = newHead;

    if (onApple2()) {
        generateApple();
    }

    if (checkDeath2()) {
        std::cout << "Game Over for Snake 2!" << std::endl;
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
    if (_nbPlayer >= 2) {
        for (const auto& segment : _snakeBody2) {
            if (segment.first == x && segment.second == y) {
                return SNAKE_2;
            }
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

int Game::getCurrentDirection2() const {
    return _currentDirection2;
}

int Game::getNbPlayer() const {
    return _nbPlayer;
}

const std::vector<std::pair<int, int>>& Game::getSnakeBody() const {
    return _snakeBody;
}

const std::vector<std::pair<int, int>>& Game::getSnakeBody2() const {
    return _snakeBody2;
}