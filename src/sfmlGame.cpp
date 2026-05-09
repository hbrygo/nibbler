#include "../includes/game.hpp"
#include "../includes/nibbler.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <chrono>

class SFMLGame {
    private:
        sf::RenderWindow* _window;
        int _width, _height;
        sf::Texture _snakeUpDownTexture;
        sf::Texture _snakeLeftRightTexture;
        sf::Texture _snakeTurnRightTexture;
        sf::Texture _snakeTurnLeftTexture;
        sf::Texture _foodTexture;
        sf::Texture _backgroundTexture1;
        sf::Texture _backgroundTexture2;
        sf::Texture _wallTexture;
        sf::Texture _snakeHeadTexture;
        sf::Texture _snakeTailTexture;
        std::chrono::high_resolution_clock::time_point _lastMoveTime;
        std::vector<std::pair<int, int>> _prevSnakeBody;

    public:
        SFMLGame(int w, int h);
        ~SFMLGame();
        void display(const Game& game);
        void display_good_part(sf::FloatRect rect, int currentDirection, const std::vector<std::pair<int, int>>& snakeBody);
        int handleInput();
};


SFMLGame::SFMLGame(int w, int h) : _window(nullptr), _width(w), _height(h), _lastMoveTime(std::chrono::high_resolution_clock::now()) {
    std::cerr << "[SFML] Initializing SFML Game..." << std::endl;

    // Create window with appropriate size
    int window_width = (w * 32 > 400) ? w * 32 : 400;
    int window_height = (h * 32 > 400) ? h * 32 : 400;

    _window = new sf::RenderWindow(sf::VideoMode(window_width, window_height), "Nibbler - SFML");
    if (!_window || !_window->isOpen()) {
        std::cerr << "[SFML] Error: Window creation failed" << std::endl;
        if (_window) delete _window;
        _window = nullptr;
        return;
    }

    std::cerr << "[SFML] Window created successfully" << std::endl;

    // Load BMP textures from textureSFML/ directory
    if (!_snakeUpDownTexture.loadFromFile("textureSFML/snake_up_down.png")) {
        std::cerr << "[SFML] Error: Unable to load snake_up_down.png" << std::endl;
    }

    if (!_snakeLeftRightTexture.loadFromFile("textureSFML/snake_left_right.png")) {
        std::cerr << "[SFML] Error: Unable to load snake_left_right.png" << std::endl;
    }

    if (!_snakeTurnRightTexture.loadFromFile("textureSFML/snake_turn_right.png")) {
        std::cerr << "[SFML] Error: Unable to load snake_turn_right.png" << std::endl;
    }

    if (!_snakeTurnLeftTexture.loadFromFile("textureSFML/snake_turn_left.png")) {
        std::cerr << "[SFML] Error: Unable to load snake_turn_left.png" << std::endl;
    }

    if (!_foodTexture.loadFromFile("textureSFML/appel_color.png")) {
        std::cerr << "[SFML] Error: Unable to load appel_color.png" << std::endl;
    }

    if (!_backgroundTexture1.loadFromFile("textureSFML/snake_ground_4.png")) {
        std::cerr << "[SFML] Error: Unable to load snake_ground_4.png" << std::endl;
    }
    
    if (!_backgroundTexture2.loadFromFile("textureSFML/snake_ground_5.png")) {
        std::cerr << "[SFML] Error: Unable to load snake_ground_5.png" << std::endl;
    }

    if (!_wallTexture.loadFromFile("textureSFML/Snake_green_wall.png")) {
        std::cerr << "[SFML] Error: Unable to load Snake_green_wall.png" << std::endl;
    }

    if (!_snakeHeadTexture.loadFromFile("textureSFML/snake_head_up.png")) {
        std::cerr << "[SFML] Error: Unable to load snake_head_up.png" << std::endl;
    }

    if (!_snakeTailTexture.loadFromFile("textureSFML/snake_tail_up.png")) {
        std::cerr << "[SFML] Error: Unable to load snake_tail_up.png" << std::endl;
    }

    std::cerr << "[SFML] BMP textures loaded successfully" << std::endl;
}

SFMLGame::~SFMLGame() {
    if (_window) {
        _window->close();
        delete _window;
    }
    std::cerr << "[SFML] SFML Game destroyed" << std::endl;
}

int getDir(int currentDirection, char axe) {
    if (currentDirection == UP) {
        return (axe == 'x') ? 0 : -1;
    } else if (currentDirection == DOWN) {
        return (axe == 'x') ? 0 : 1;
    } else if (currentDirection == LEFT) {
        return (axe == 'x') ? -1 : 0;
    } else if (currentDirection == RIGHT) {
        return (axe == 'x') ? 1 : 0;
    }
    return 0;
}

void SFMLGame::display_good_part(sf::FloatRect rect, int currentDirection, const std::vector<std::pair<int, int>>& snakeBody) {
    (void)rect;
    
    // Initialiser _prevSnakeBody au premier appel
    if (_prevSnakeBody.empty()) {
        _prevSnakeBody = snakeBody;
        _lastMoveTime = std::chrono::high_resolution_clock::now();
    }
    
    auto now = std::chrono::high_resolution_clock::now();
    
    // Détecter si le serpent a bougé
    if (_prevSnakeBody != snakeBody) {
        _lastMoveTime = now;
        _prevSnakeBody = snakeBody;
    }
    
    // Calculer le temps écoulé depuis le dernier mouvement (en millisecondes)
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastMoveTime).count();
    float progress = std::min(1.0f, elapsed / 150.0f);  // TICK_RATE = 150ms
    
    auto segmentAngleFromDelta = [](int dx, int dy) -> float {
        if (dx == 1) return 90.0f;
        if (dx == -1) return 270.0f;
        if (dy == 1) return 180.0f;
        if (dy == -1) return 0.0f;
        return 0.0f;
    };

    auto directionAngle = [](int direction) -> float {
        if (direction == RIGHT) return 90.0f;
        if (direction == DOWN) return 180.0f;
        if (direction == LEFT) return 270.0f;
        return 0.0f;
    };

    for (size_t i = 0; i < snakeBody.size(); ++i) {
        // Calculer la position interpolée du segment
        float centerX, centerY;
        if (i < _prevSnakeBody.size()) {
            float oldX = _prevSnakeBody[i].first * 32.0f + 16.0f;
            float oldY = _prevSnakeBody[i].second * 32.0f + 16.0f;
            float newX = snakeBody[i].first * 32.0f + 16.0f;
            float newY = snakeBody[i].second * 32.0f + 16.0f;
            centerX = oldX + (newX - oldX) * progress;
            centerY = oldY + (newY - oldY) * progress;
        } else {
            centerX = snakeBody[i].first * 32.0f + 16.0f;
            centerY = snakeBody[i].second * 32.0f + 16.0f;
        }

        // HEAD
        if (i == 0) {
            sf::Sprite headSprite(_snakeHeadTexture);
            headSprite.setPosition(centerX, centerY);
            headSprite.setOrigin(16.0f + (0.25 * getDir(currentDirection, 'x')), 16.0f + (0.25 * getDir(currentDirection, 'y')));
            headSprite.setRotation(directionAngle(currentDirection));
            _window->draw(headSprite);
        }
        // TAIL
        else if (i == snakeBody.size() - 1) {
            auto prev = snakeBody[i - 1];
            auto tail = snakeBody[i];
            int dx = tail.first - prev.first;
            int dy = tail.second - prev.second;
            
            sf::Sprite tailSprite(_snakeTailTexture);
            tailSprite.setPosition(centerX, centerY);
            tailSprite.setOrigin(16.0f, 16.0f);
            tailSprite.setRotation(segmentAngleFromDelta(-dx, -dy));
            _window->draw(tailSprite);
        }
        // BODY
        else {
            auto prev = snakeBody[i - 1];
            auto curr = snakeBody[i];
            auto next = snakeBody[i + 1];

            int dx1 = curr.first - prev.first;
            int dy1 = curr.second - prev.second;
            int dx2 = next.first - curr.first;
            int dy2 = next.second - curr.second;

            // STRAIGHT LINE
            if ((dx1 == dx2) && (dy1 == dy2)) {
                sf::Sprite bodySprite(_snakeUpDownTexture);
                bodySprite.setPosition(centerX, centerY);
                bodySprite.setOrigin(16.0f, 16.0f);
                bodySprite.setRotation(segmentAngleFromDelta(-dx1, -dy1));
                _window->draw(bodySprite);
            }
            // TURN
            else {
                sf::Texture* texture = &_snakeTurnLeftTexture;
                float angle = 0.0f;

                if (dx1 == 0 && dy1 == -1 && dx2 == -1 && dy2 == 0) {
                    angle = 270.0f;
                }
                else if (dx1 == -1 && dy1 == 0 && dx2 == 0 && dy2 == 1) {
                    angle = 180.0f;
                }
                else if (dx1 == 0 && dy1 == 1 && dx2 == 1 && dy2 == 0) {
                    angle = 90.0f;
                }
                else if (dx1 == 1 && dy1 == 0 && dx2 == 0 && dy2 == -1) {
                    angle = 0.0f;
                }
                else if (dx2 == 0 && dy2 == -1 && dx1 == -1 && dy1 == 0) {
                    angle = 90.0f;
                }
                else if (dx2 == -1 && dy2 == 0 && dx1 == 0 && dy1 == 1) {
                    angle = 0.0f;
                }
                else if (dx2 == 1 && dy2 == 0 && dx1 == 0 && dy1 == -1) {
                    angle = 180.0f;
                }
                else if (dx2 == 0 && dy2 == 1 && dx1 == 1 && dy1 == 0) {
                    angle = 270.0f;
                }

                sf::Sprite turnSprite(*texture);
                turnSprite.setPosition(centerX, centerY);
                turnSprite.setOrigin(16.0f, 16.0f);
                turnSprite.setRotation(angle);
                _window->draw(turnSprite);
            }
        }
    }
}

void SFMLGame::display(const Game& game) {
    if (!_window || !_window->isOpen()) {
        return;
    }
    _window->clear(sf::Color::Black);
    
    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            if ((i + j) % 2 == 0) {
                sf::Sprite backgroundSprite(_backgroundTexture1);
                backgroundSprite.setPosition(i * 32.0f, j * 32.0f);
                _window->draw(backgroundSprite);
            } else {
                sf::Sprite backgroundSprite(_backgroundTexture2);
                backgroundSprite.setPosition(i * 32.0f, j * 32.0f);
                _window->draw(backgroundSprite);
            }

            int cell = game.getCell(i, j);
            if (cell == FOOD) {
                sf::Sprite foodSprite(_foodTexture);
                foodSprite.setPosition((i * 32.0f) + 16.0f, (j * 32.0f) + 16.0f);
                foodSprite.setOrigin(16.0f, 16.0f);
                _window->draw(foodSprite);
            } else if (cell == WALL) {
                sf::Sprite wallSprite(_wallTexture);
                wallSprite.setPosition((i * 32.0f) + 16.0f, (j * 32.0f) + 16.0f);
                wallSprite.setOrigin(16.0f, 16.0f);
                _window->draw(wallSprite);
            }
        }
    }

    // AFFICHER LE SERPENT UNE SEULE FOIS, EN DEHORS DE LA GRILLE
    if (_snakeUpDownTexture.getSize().x > 0 && _snakeLeftRightTexture.getSize().x > 0 && _snakeTurnRightTexture.getSize().x > 0 && _snakeTurnLeftTexture.getSize().x > 0) {
        sf::FloatRect snakeRect = { 0.0f, 0.0f, 32.0f, 32.0f };
        display_good_part(snakeRect, game.getCurrentDirection(), game.getSnakeBody());
    }

    _window->display();
}

int SFMLGame::handleInput() {
    if (!_window || !_window->isOpen()) {
        return 0;
    }

    sf::Event event;
    while (_window->pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            std::cerr << "[SFML] Window closed event received" << std::endl;
            return -1;
        }

        if (event.type == sf::Event::KeyPressed) {
            std::cerr << "[SFML] KEY_PRESSED - code: " << event.key.code << std::endl;

            if (event.key.code == sf::Keyboard::Escape) {
                std::cerr << "[INPUT] ESCAPE detected -> returning -1" << std::endl;
                return -1;
            }
            // Arrow keys
            if (event.key.code == sf::Keyboard::Left) {
                std::cerr << "[INPUT] LEFT detected -> returning " << LEFT << std::endl;
                return LEFT;
            }
            if (event.key.code == sf::Keyboard::Right) {
                std::cerr << "[INPUT] RIGHT detected -> returning " << RIGHT << std::endl;
                return RIGHT;
            }
            if (event.key.code == sf::Keyboard::Up) {
                std::cerr << "[INPUT] UP detected -> returning " << UP << std::endl;
                return UP;
            }
            if (event.key.code == sf::Keyboard::Down) {
                std::cerr << "[INPUT] DOWN detected -> returning " << DOWN << std::endl;
                return DOWN;
            }
            // Mode controls (use values > 4 to avoid conflict with Direction enum)
            if (event.key.code == sf::Keyboard::Num1) {
                std::cerr << "[INPUT] Mode 1" << std::endl;
                currentLibrary = SFML;
                return 10;
            }
            if (event.key.code == sf::Keyboard::Num2) {
                std::cerr << "[INPUT] Mode 2" << std::endl;
                currentLibrary = SDL3;
                return 20;
            }
            if (event.key.code == sf::Keyboard::Num3) {
                std::cerr << "[INPUT] Mode 3" << std::endl;
                currentLibrary = GL;
                return 30;
            }
        }
    }

    return 0;
}

extern "C" {
    void* create_gui_sfml(int width, int height) {
        return new SFMLGame(width, height);
    }

    void destroy_gui_sfml(void* gui) {
        delete (SFMLGame*)gui;
    }

    void display_gui_sfml(void* gui, const Game& game) {
        ((SFMLGame*)gui)->display(game);
    }

    int input_gui_sfml(void* gui) {
        return ((SFMLGame*)gui)->handleInput();
    }
}