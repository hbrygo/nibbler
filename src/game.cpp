#include "nibbler.hpp"

Game::Game() : Game(10, 10, 1, false, false, false) {
}

Game::Game(int height, int width, bool michaelMode, bool despawnApple, bool wallMode) : Game(height, width, 1, michaelMode, despawnApple, wallMode) {
}

Game::Game(int height, int width, int nbPlayer, bool michaelMode, bool despawnApple, bool wallMode) : _gameAreaHeight(height), _gameAreaWidth(width), _wallPosition({-1, -1}), _despawnApple(despawnApple), _score(0), _michaelMode(michaelMode), _wallMode(wallMode) {
    _nbPlayer = nbPlayer < 1 ? 1 : nbPlayer;
    _gameArea.resize(_gameAreaHeight, std::vector<CellType>(_gameAreaWidth, EMPTY));
    if (_wallMode) {
        int nbr_wall = (_gameAreaHeight + _gameAreaWidth) / 10;

        for (int i = 0; i < nbr_wall; ++i) {
            int x = rand() % _gameAreaHeight;
            int y = rand() % _gameAreaWidth;
            _gameArea[x][y] = WALL;
        }
    }
    _snakeSize = 4;
    _snakeSize2 = 4;
    for (int i = 0; i < _snakeSize; ++i) {
        _snakeBody.push_back({_gameAreaHeight / 2, _gameAreaWidth / 2 - i});
    }
    if (_nbPlayer >= 2) {
        int rightHeadCol = std::max(_gameAreaWidth - 4, 4);
        for (int i = 0; i < _snakeSize2; ++i) {
            _snakeBody2.push_back({_gameAreaHeight / 2 + 1, rightHeadCol + i});
        }
        _currentDirection2 = LEFT;
    }
    _currentDirection = RIGHT;
    generateApple();
}

Game::Game(const Game& other) : _gameAreaHeight(other._gameAreaHeight), _gameAreaWidth(other._gameAreaWidth), _gameArea(other._gameArea), _snakeSize(other._snakeSize), _snakeSize2(other._snakeSize2), _nbPlayer(other._nbPlayer), _snakeBody(other._snakeBody), _snakeBody2(other._snakeBody2), _currentDirection(other._currentDirection), _currentDirection2(other._currentDirection2), _applePosition(other._applePosition), _wallPosition(other._wallPosition), _spawnApple(other._spawnApple), _despawnApple(other._despawnApple), _score(other._score), _michaelMode(other._michaelMode), _wallMode(other._wallMode) {}

Game& Game::operator=(const Game& other) {
    if (this != &other) {
        _gameAreaHeight = other._gameAreaHeight;
        _gameAreaWidth = other._gameAreaWidth;
        _gameArea = other._gameArea;
        _snakeSize = other._snakeSize;
        _snakeSize2 = other._snakeSize2;
        _nbPlayer = other._nbPlayer;
        _snakeBody = other._snakeBody;
        _snakeBody2 = other._snakeBody2;
        _currentDirection = other._currentDirection;
        _currentDirection2 = other._currentDirection2;
        _applePosition = other._applePosition;
        _wallPosition = other._wallPosition;
        _spawnApple = other._spawnApple;
        _despawnApple = other._despawnApple;
        _score = other._score;
        _michaelMode = other._michaelMode;
        _wallMode = other._wallMode;
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
                if (_nbPlayer >= 2) {
                    for (const auto &segment : _snakeBody2) {
                        if (segment.first == i && segment.second == j) {
                            std::cout << "S ";
                            printed = true;
                            break;
                        }
                    }
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
    _currentDirection = direction;
}

void Game::changeDirection2(Direction direction) {
    if ((getCurrentDirection2() == UP && direction == DOWN) ||
        (getCurrentDirection2() == DOWN && direction == UP) ||
        (getCurrentDirection2() == LEFT && direction == RIGHT) ||
        (getCurrentDirection2() == RIGHT && direction == LEFT)) {
        std::cerr << "Snake 2 is already moving in that direction!" << std::endl;
        return;
    }
    _currentDirection2 = direction;
}

int Game::checkDeath() {
    const auto& head = _snakeBody.front();
    // wall
    if (head.first < 0 || head.first >= _gameAreaHeight || head.second < 0 || head.second >= _gameAreaWidth || head == _wallPosition) {
        return -1;
    }
    // snake
    for (size_t i = 1; i < _snakeBody.size(); ++i) {
        if (head == _snakeBody[i]) {
            return -1;
        }
    }
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
    if (_nbPlayer < 2 || _snakeBody2.empty()) {
        return 0;
    }

    const auto& head = _snakeBody2.front();
    if (head.first < 0 || head.first >= _gameAreaHeight || head.second < 0 || head.second >= _gameAreaWidth || head == _wallPosition) {
        return -1;
    }
    for (size_t i = 1; i < _snakeBody2.size(); ++i) {
        if (head == _snakeBody2[i]) {
            return -1;
        }
    }
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
        std::pair<int, int> newSegment = _snakeBody.back();
        std::pair<int, int> previous = _snakeBody[_snakeBody.size() - 2];
        if (previous.first == newSegment.first && previous.second == newSegment.second + 1) {
            newSegment.second += 1;
        } else if (previous.first == newSegment.first && previous.second == newSegment.second - 1) {
            newSegment.second -= 1;
        } else if (previous.first == newSegment.first + 1 && previous.second == newSegment.second) {
            newSegment.first += 1;
        } else if (previous.first == newSegment.first - 1 && previous.second == newSegment.second) {
            newSegment.first -= 1;
        }
        _snakeBody.push_back(newSegment);
        return 1;
    }
    return 0;
}

int Game::onApple2() {
    if (_nbPlayer < 2 || _snakeBody2.empty()) {
        return 0;
    }

    const auto& head = _snakeBody2.front();
    if (head == _applePosition) {
        _snakeSize2++;
        std::pair<int, int> newSegment = _snakeBody2.back();
        std::pair<int, int> previous = _snakeBody2[_snakeBody2.size() - 2];
        if (previous.first == newSegment.first && previous.second == newSegment.second + 1) {
            newSegment.second += 1;
        } else if (previous.first == newSegment.first && previous.second == newSegment.second - 1) {
            newSegment.second -= 1;
        } else if (previous.first == newSegment.first + 1 && previous.second == newSegment.second) {
            newSegment.first += 1;
        } else if (previous.first == newSegment.first - 1 && previous.second == newSegment.second) {
            newSegment.first -= 1;
        }
        _snakeBody2.push_back(newSegment);
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

    for (int i = 0; i < _gameAreaHeight; ++i) {
        for (int j = 0; j < _gameAreaWidth; ++j) {
            if ((_wallPosition.first == i && _wallPosition.second == j) || std::find(_snakeBody.begin(), _snakeBody.end(), std::make_pair(i, j)) != _snakeBody.end() ||
                (_nbPlayer >= 2 && std::find(_snakeBody2.begin(), _snakeBody2.end(), std::make_pair(i, j)) != _snakeBody2.end())) {
                occupiedCells++;
            }
        }
    }

    if (occupiedCells >= static_cast<size_t>(_gameAreaHeight) * static_cast<size_t>(_gameAreaWidth)) {
        _applePosition = {-1, -1};
        return;
    }

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> rowDist(0, _gameAreaHeight - 1);
    std::uniform_int_distribution<int> colDist(0, _gameAreaWidth - 1);

    std::pair<int,int> pos;
    do {
        pos = { rowDist(rng), colDist(rng) };
    } while (std::find(_snakeBody.begin(), _snakeBody.end(), pos) != _snakeBody.end() ||
        (_nbPlayer >= 2 && std::find(_snakeBody2.begin(), _snakeBody2.end(), pos) != _snakeBody2.end()) ||
        pos == _wallPosition);

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
        std::cerr << "This apple give you " << 100 - elapsed << " points!" << std::endl;
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

int Game::moveSnake2(int& onAppleSound, std::mutex& onAppleMutex) {
    if (_nbPlayer < 2 || _snakeBody2.empty()) {
        return 0;
    }

    std::pair<int, int> newHead = _snakeBody2.front();

    if (_currentDirection2 == UP) {
        newHead.second -= 1;
    } else if (_currentDirection2 == DOWN) {
        newHead.second += 1;
    } else if (_currentDirection2 == LEFT) {
        newHead.first -= 1;
    } else if (_currentDirection2 == RIGHT) {
        newHead.first += 1;
    } else {
        return 0;
    }

    for (size_t i = _snakeBody2.size() - 1; i > 0; --i) {
        _snakeBody2[i] = _snakeBody2[i - 1];
    }
    _snakeBody2[0] = newHead;

    if (onApple2()) {
        struct timeval now;
        gettimeofday(&now, nullptr);
        long elapsed = (now.tv_sec - _spawnApple.tv_sec) * 2;
        if (elapsed > 100) elapsed = 100;
        _score += 100 - elapsed;
        std::cerr << "This apple give you " << 100 - elapsed << " points!" << std::endl;
        {
            std::lock_guard<std::mutex> lock(onAppleMutex);
            onAppleSound++;
        }
        generateApple();
    }

    if (checkDeath2()) {
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

const std::vector<std::pair<int, int>>& Game::getSnakeBody() const {
    return _snakeBody;
}

std::vector<std::pair<int, int>>& Game::getSnakeBody() {
    return _snakeBody;
}

std::vector<std::pair<int, int>>& Game::getSnakeBody2() {
    return _snakeBody2;
}

const std::vector<std::pair<int, int>>& Game::getSnakeBody2() const {
    return _snakeBody2;
}

void Game::setMichaelMode(bool enabled) {
    _michaelMode = enabled;
}

bool Game::getMichaelMode() const {
    return _michaelMode;
}

timeval Game::getSpawnApple() const {
    return _spawnApple;
}

bool Game::getAppelDespawned() const {
    return _despawnApple;
}

int Game::getWidth() const {
    return _gameAreaWidth;
}

int Game::getHeight() const {
    return _gameAreaHeight;
}

int Game::getNbPlayer() const {
    return _nbPlayer;
}