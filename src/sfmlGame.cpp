#include "../includes/SfmlGame.hpp"

typedef sf::RenderWindow* (*PFSFMLCREATERENDERWINDOWPROC)(int, int, const char*);
typedef void (*PFSFMLDESTROYRENDERWINDOWPROC)(sf::RenderWindow*);
typedef bool (*PFSFMLWINDOWISOPENPROC)(sf::RenderWindow*);
typedef void (*PFSFMLWINDOWCLOSEPROC)(sf::RenderWindow*);
typedef void (*PFSFMLWINDOWCLEARPROC)(sf::RenderWindow*, unsigned char, unsigned char, unsigned char, unsigned char);
typedef bool (*PFSFMLWINDOWPOLLEVENTPROC)(sf::RenderWindow*, sf::Event*);
typedef void (*PFSFMLWINDOWDRAWSPRITEPROC)(sf::RenderWindow*, const sf::Sprite*);
typedef void (*PFSFMLWINDOWDISPLAYPROC)(sf::RenderWindow*);
typedef sf::Texture* (*PFSFMLLOADTEXTUREPROC)(const char*);
typedef void (*PFSFMLDESTROYTEXTUREPROC)(sf::Texture*);

static PFSFMLCREATERENDERWINDOWPROC SFML_CreateRenderWindow_ptr = nullptr;
static PFSFMLDESTROYRENDERWINDOWPROC SFML_DestroyRenderWindow_ptr = nullptr;
static PFSFMLWINDOWISOPENPROC SFML_WindowIsOpen_ptr = nullptr;
static PFSFMLWINDOWCLOSEPROC SFML_WindowClose_ptr = nullptr;
static PFSFMLWINDOWCLEARPROC SFML_WindowClear_ptr = nullptr;
static PFSFMLWINDOWPOLLEVENTPROC SFML_WindowPollEvent_ptr = nullptr;
static PFSFMLWINDOWDRAWSPRITEPROC SFML_WindowDrawSprite_ptr = nullptr;
static PFSFMLWINDOWDISPLAYPROC SFML_WindowDisplay_ptr = nullptr;
static PFSFMLLOADTEXTUREPROC SFML_LoadTexture_ptr = nullptr;
static PFSFMLDESTROYTEXTUREPROC SFML_DestroyTexture_ptr = nullptr;
static void* sfml_handle = nullptr;
static bool initialized = false;

static bool load_sfml_symbols() {
    if (initialized) {
        return true;
    }

    Dl_info info;
    if (!dladdr(reinterpret_cast<void*>(&load_sfml_symbols), &info) || !info.dli_fname) {
        std::cerr << "[SFML] Error: unable to resolve current module for symbol lookup" << std::endl;
        return false;
    }

    sfml_handle = dlopen(info.dli_fname, RTLD_NOW | RTLD_NOLOAD);
    if (!sfml_handle) {
        std::cerr << "[SFML] Error: unable to open module handle - " << dlerror() << std::endl;
        return false;
    }

    SFML_CreateRenderWindow_ptr = reinterpret_cast<PFSFMLCREATERENDERWINDOWPROC>(dlsym(sfml_handle, "sfml_create_render_window"));
    SFML_DestroyRenderWindow_ptr = reinterpret_cast<PFSFMLDESTROYRENDERWINDOWPROC>(dlsym(sfml_handle, "sfml_destroy_render_window"));
    SFML_WindowIsOpen_ptr = reinterpret_cast<PFSFMLWINDOWISOPENPROC>(dlsym(sfml_handle, "sfml_window_is_open"));
    SFML_WindowClose_ptr = reinterpret_cast<PFSFMLWINDOWCLOSEPROC>(dlsym(sfml_handle, "sfml_window_close"));
    SFML_WindowClear_ptr = reinterpret_cast<PFSFMLWINDOWCLEARPROC>(dlsym(sfml_handle, "sfml_window_clear"));
    SFML_WindowPollEvent_ptr = reinterpret_cast<PFSFMLWINDOWPOLLEVENTPROC>(dlsym(sfml_handle, "sfml_window_poll_event"));
    SFML_WindowDrawSprite_ptr = reinterpret_cast<PFSFMLWINDOWDRAWSPRITEPROC>(dlsym(sfml_handle, "sfml_window_draw_sprite"));
    SFML_WindowDisplay_ptr = reinterpret_cast<PFSFMLWINDOWDISPLAYPROC>(dlsym(sfml_handle, "sfml_window_display"));
    SFML_LoadTexture_ptr = reinterpret_cast<PFSFMLLOADTEXTUREPROC>(dlsym(sfml_handle, "sfml_load_texture"));
    SFML_DestroyTexture_ptr = reinterpret_cast<PFSFMLDESTROYTEXTUREPROC>(dlsym(sfml_handle, "sfml_destroy_texture"));

    if (!SFML_CreateRenderWindow_ptr || !SFML_DestroyRenderWindow_ptr || !SFML_WindowIsOpen_ptr ||
        !SFML_WindowClose_ptr || !SFML_WindowClear_ptr || !SFML_WindowPollEvent_ptr ||
        !SFML_WindowDrawSprite_ptr || !SFML_WindowDisplay_ptr || !SFML_LoadTexture_ptr ||
        !SFML_DestroyTexture_ptr) {
        std::cerr << "[SFML] Error: unable to load all SFML wrapper symbols" << std::endl;
        return false;
    }

    initialized = true;
    std::cerr << "[SFML] SFML wrapper symbols loaded successfully" << std::endl;
    return true;
}

SFMLGame::SFMLGame() : _window(nullptr), _width(0), _height(0), _michaelMode(false),
    _snakeUpDownTexture(nullptr), _snakeLeftRightTexture(nullptr), _snakeTurnRightTexture(nullptr),
    _snakeTurnLeftTexture(nullptr), _foodTexture(nullptr), _backgroundTexture1(nullptr),
    _backgroundTexture2(nullptr), _wallTexture(nullptr), _snakeHeadTexture(nullptr),
    _snakeTailTexture(nullptr), _michaelModeTexture(nullptr) {
}

SFMLGame::SFMLGame(const SFMLGame& other) : _window(other._window), _width(other._width), _height(other._height), _michaelMode(other._michaelMode),
    _snakeUpDownTexture(other._snakeUpDownTexture), _snakeLeftRightTexture(other._snakeLeftRightTexture),
    _snakeTurnRightTexture(other._snakeTurnRightTexture), _snakeTurnLeftTexture(other._snakeTurnLeftTexture),
    _foodTexture(other._foodTexture), _backgroundTexture1(other._backgroundTexture1),
    _backgroundTexture2(other._backgroundTexture2), _wallTexture(other._wallTexture),
    _snakeHeadTexture(other._snakeHeadTexture), _snakeTailTexture(other._snakeTailTexture), _michaelModeTexture(other._michaelModeTexture) {
}

SFMLGame& SFMLGame::operator=(const SFMLGame& other) {
    if (this != &other) {
        _window = other._window;
        _width = other._width;
        _height = other._height;
        _michaelMode = other._michaelMode;
         _snakeUpDownTexture = other._snakeUpDownTexture;
        _snakeLeftRightTexture = other._snakeLeftRightTexture;
        _snakeTurnRightTexture = other._snakeTurnRightTexture;
        _snakeTurnLeftTexture = other._snakeTurnLeftTexture;
        _foodTexture = other._foodTexture;
        _backgroundTexture1 = other._backgroundTexture1;
        _backgroundTexture2 = other._backgroundTexture2;
        _wallTexture = other._wallTexture;
        _snakeHeadTexture = other._snakeHeadTexture;
        _snakeTailTexture = other._snakeTailTexture;
        _michaelModeTexture = other._michaelModeTexture;
    }
    return *this;
}

SFMLGame::SFMLGame(int w, int h, bool michaelMode) : _window(nullptr), _width(w), _height(h), _michaelMode(michaelMode),
    _snakeUpDownTexture(nullptr), _snakeLeftRightTexture(nullptr), _snakeTurnRightTexture(nullptr),
    _snakeTurnLeftTexture(nullptr), _foodTexture(nullptr), _backgroundTexture1(nullptr),
    _backgroundTexture2(nullptr), _wallTexture(nullptr), _snakeHeadTexture(nullptr),
    _snakeTailTexture(nullptr), _michaelModeTexture(nullptr) {
    std::cerr << "[SFML] Initializing SFML Game..." << std::endl;

    if (!load_sfml_symbols()) {
        return;
    }

    // Create window with appropriate size
    int window_width = 0;
    int window_height = 0;
    if (michaelMode) {
        window_width = 1400;
        window_height = 1800;
    } else {
        window_width = (w * 32 > 400) ? w * 32 : 400;
        window_height = (h * 32 > 400) ? h * 32 : 400;
    }

    _window = SFML_CreateRenderWindow_ptr(window_width, window_height, "Nibbler - SFML");
    if (!_window || !SFML_WindowIsOpen_ptr(_window)) {
        std::cerr << "[SFML] Error: Window creation failed" << std::endl;
        if (_window) {
            SFML_DestroyRenderWindow_ptr(_window);
        }
        _window = nullptr;
        return;
    }

    std::cerr << "[SFML] Window created successfully" << std::endl;

    _snakeUpDownTexture = SFML_LoadTexture_ptr("textureSFML/snake_up_down.png");
    if (!_snakeUpDownTexture) {
        std::cerr << "[SFML] Error: Unable to load snake_up_down.png" << std::endl;
    }

    _snakeLeftRightTexture = SFML_LoadTexture_ptr("textureSFML/snake_left_right.png");
    if (!_snakeLeftRightTexture) {
        std::cerr << "[SFML] Error: Unable to load snake_left_right.png" << std::endl;
    }

    _snakeTurnRightTexture = SFML_LoadTexture_ptr("textureSFML/snake_turn_right.png");
    if (!_snakeTurnRightTexture) {
        std::cerr << "[SFML] Error: Unable to load snake_turn_right.png" << std::endl;
    }

    _snakeTurnLeftTexture = SFML_LoadTexture_ptr("textureSFML/snake_turn_left.png");
    if (!_snakeTurnLeftTexture) {
        std::cerr << "[SFML] Error: Unable to load snake_turn_left.png" << std::endl;
    }

    _foodTexture = SFML_LoadTexture_ptr("textureSFML/appel_color.png");
    if (!_foodTexture) {
        std::cerr << "[SFML] Error: Unable to load appel_color.png" << std::endl;
    }

    _backgroundTexture1 = SFML_LoadTexture_ptr("textureSFML/snake_ground_4.png");
    if (!_backgroundTexture1) {
        std::cerr << "[SFML] Error: Unable to load snake_ground_4.png" << std::endl;
    }
    
    _backgroundTexture2 = SFML_LoadTexture_ptr("textureSFML/snake_ground_5.png");
    if (!_backgroundTexture2) {
        std::cerr << "[SFML] Error: Unable to load snake_ground_5.png" << std::endl;
    }

    _wallTexture = SFML_LoadTexture_ptr("textureSFML/Snake_green_wall.png");
    if (!_wallTexture) {
        std::cerr << "[SFML] Error: Unable to load Snake_green_wall.png" << std::endl;
    }

    _snakeHeadTexture = SFML_LoadTexture_ptr("textureSFML/snake_head_up.png");
    if (!_snakeHeadTexture) {
        std::cerr << "[SFML] Error: Unable to load snake_head_up.png" << std::endl;
    }

    _snakeTailTexture = SFML_LoadTexture_ptr("textureSFML/snake_tail_up.png");
    if (!_snakeTailTexture) {
        std::cerr << "[SFML] Error: Unable to load snake_tail_up.png" << std::endl;
    }

    _michaelModeTexture = SFML_LoadTexture_ptr("textureSFML/projet_michael_youn.png");
    if (!_michaelModeTexture) {
        std::cerr << "[SFML] Error: Unable to load projet_michael_youn.png" << std::endl;
    }

    std::cerr << "[SFML] BMP textures loaded successfully" << std::endl;
}

SFMLGame::~SFMLGame() {
    if (_window) {
        SFML_WindowClose_ptr(_window);
        SFML_DestroyRenderWindow_ptr(_window);
    }
    if (_snakeUpDownTexture) SFML_DestroyTexture_ptr(_snakeUpDownTexture);
    if (_snakeLeftRightTexture) SFML_DestroyTexture_ptr(_snakeLeftRightTexture);
    if (_snakeTurnRightTexture) SFML_DestroyTexture_ptr(_snakeTurnRightTexture);
    if (_snakeTurnLeftTexture) SFML_DestroyTexture_ptr(_snakeTurnLeftTexture);
    if (_foodTexture) SFML_DestroyTexture_ptr(_foodTexture);
    if (_backgroundTexture1) SFML_DestroyTexture_ptr(_backgroundTexture1);
    if (_backgroundTexture2) SFML_DestroyTexture_ptr(_backgroundTexture2);
    if (_wallTexture) SFML_DestroyTexture_ptr(_wallTexture);
    if (_snakeHeadTexture) SFML_DestroyTexture_ptr(_snakeHeadTexture);
    if (_snakeTailTexture) SFML_DestroyTexture_ptr(_snakeTailTexture);
    if (_michaelModeTexture) SFML_DestroyTexture_ptr(_michaelModeTexture);
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

void SFMLGame::display_good_part(int currentDirection, const std::vector<std::pair<int, int>>& snakeBody, bool modeMichael) {
    const float offsetX = modeMichael ? 640.0f : 0.0f;
    const float offsetY = modeMichael ? 350.0f : 0.0f;
    
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
            float oldX = (_prevSnakeBody[i].first * 32.0f + 16.0f) + offsetX;
            float oldY = (_prevSnakeBody[i].second * 32.0f + 16.0f) + offsetY;
            float newX = (snakeBody[i].first * 32.0f + 16.0f) + offsetX;
            float newY = (snakeBody[i].second * 32.0f + 16.0f) + offsetY;
            centerX = oldX + (newX - oldX) * progress;
            centerY = oldY + (newY - oldY) * progress;
        } else {
            centerX = (snakeBody[i].first * 32.0f + 16.0f) + offsetX;
            centerY = (snakeBody[i].second * 32.0f + 16.0f) + offsetY;
        }

        // HEAD
        if (i == 0) {
            sf::Sprite headSprite(*_snakeHeadTexture);
            headSprite.setPosition(centerX, centerY);
            headSprite.setOrigin(16.0f + (0.25 * getDir(currentDirection, 'x')), 16.0f + (0.25 * getDir(currentDirection, 'y')));
            headSprite.setRotation(directionAngle(currentDirection));
            SFML_WindowDrawSprite_ptr(_window, &headSprite);
        }
        // TAIL
        else if (i == snakeBody.size() - 1) {
            auto prev = snakeBody[i - 1];
            auto tail = snakeBody[i];
            int dx = tail.first - prev.first;
            int dy = tail.second - prev.second;
            
            sf::Sprite tailSprite(*_snakeTailTexture);
            tailSprite.setPosition(centerX, centerY);
            tailSprite.setOrigin(16.0f, 16.0f);
            tailSprite.setRotation(segmentAngleFromDelta(-dx, -dy));
            SFML_WindowDrawSprite_ptr(_window, &tailSprite);
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
                sf::Sprite bodySprite(*_snakeUpDownTexture);
                bodySprite.setPosition(centerX, centerY);
                bodySprite.setOrigin(16.0f, 16.0f);
                bodySprite.setRotation(segmentAngleFromDelta(-dx1, -dy1));
                SFML_WindowDrawSprite_ptr(_window, &bodySprite);
            }
            // TURN
            else {
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

                sf::Sprite turnSprite(*_snakeTurnLeftTexture);
                turnSprite.setPosition(centerX, centerY);
                turnSprite.setOrigin(16.0f, 16.0f);
                turnSprite.setRotation(angle);
                SFML_WindowDrawSprite_ptr(_window, &turnSprite);
            }
        }
    }
}

void SFMLGame::display(const Game& game) {
    const float offsetX = game.getMichaelMode() ? 640.0f : 0.0f;
    const float offsetY = game.getMichaelMode() ? 350.0f : 0.0f;

    if (!_window || !SFML_WindowIsOpen_ptr(_window)) {
        return;
    }
    SFML_WindowClear_ptr(_window, 0, 0, 0, 255);
    
    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            if ((i + j) % 2 == 0) {
                sf::Sprite backgroundSprite(*_backgroundTexture1);
                backgroundSprite.setPosition((i * 32.0f) + offsetX, (j * 32.0f) + offsetY);
                SFML_WindowDrawSprite_ptr(_window, &backgroundSprite);
            } else {
                sf::Sprite backgroundSprite(*_backgroundTexture2);
                backgroundSprite.setPosition((i * 32.0f) + offsetX, (j * 32.0f) + offsetY);
                SFML_WindowDrawSprite_ptr(_window, &backgroundSprite);
            }

            int cell = game.getCell(i, j);
            if (cell == FOOD) {
                sf::Sprite foodSprite(*_foodTexture);
                foodSprite.setPosition(((i * 32.0f) + 16.0f) + offsetX, ((j * 32.0f) + 16.0f) + offsetY);
                foodSprite.setOrigin(16.0f, 16.0f);
                SFML_WindowDrawSprite_ptr(_window, &foodSprite);
            } else if (cell == WALL) {
                sf::Sprite wallSprite(*_wallTexture);
                wallSprite.setPosition((i * 32.0f) + 16.0f + offsetX, ((j * 32.0f) + 16.0f) + offsetY);
                wallSprite.setOrigin(16.0f, 16.0f);
                SFML_WindowDrawSprite_ptr(_window, &wallSprite);
            }
        }
    }

    // AFFICHER LE SERPENT UNE SEULE FOIS, EN DEHORS DE LA GRILLE
    if (_snakeUpDownTexture && _snakeLeftRightTexture && _snakeTurnRightTexture && _snakeTurnLeftTexture) {
        sf::FloatRect snakeRect = { 0.0f, 0.0f, 32.0f, 32.0f };
        display_good_part(game.getCurrentDirection(), game.getSnakeBody(), game.getMichaelMode());
    }

    if (game.getMichaelMode() && _michaelModeTexture) {
        sf::Sprite michaelSprite(*_michaelModeTexture);
        michaelSprite.setPosition(0.0f, 0.0f);
        michaelSprite.setOrigin(0.0f, 0.0f);
        const sf::Vector2u windowSize = _window->getSize();
        const sf::Vector2u textureSize = _michaelModeTexture->getSize();
        if (textureSize.x != 0 && textureSize.y != 0) {
            michaelSprite.setScale(
                static_cast<float>(windowSize.x) / static_cast<float>(textureSize.x),
                static_cast<float>(windowSize.y) / static_cast<float>(textureSize.y)
            );
        }
        SFML_WindowDrawSprite_ptr(_window, &michaelSprite);
    }

    SFML_WindowDisplay_ptr(_window);
}

int SFMLGame::handleInput() {
    if (!_window || !SFML_WindowIsOpen_ptr(_window)) {
        return 0;
    }

    sf::Event event;
    while (SFML_WindowPollEvent_ptr(_window, &event)) {
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
                currentLibrary = SDL3;
                return 10;
            }
            if (event.key.code == sf::Keyboard::Num2) {
                std::cerr << "[INPUT] Mode 2" << std::endl;
                currentLibrary = SFML;
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
    void* create_gui_sfml(int width, int height, bool michaelMode) {
        return new SFMLGame(width, height, michaelMode);
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

    sf::RenderWindow* sfml_create_render_window(int width, int height, const char* title) {
        return new sf::RenderWindow(sf::VideoMode(width, height), title);
    }

    void sfml_destroy_render_window(sf::RenderWindow* window) {
        if (window) {
            delete window;
        }
    }

    bool sfml_window_is_open(sf::RenderWindow* window) {
        return window && window->isOpen();
    }

    void sfml_window_close(sf::RenderWindow* window) {
        if (window) {
            window->close();
        }
    }

    void sfml_window_clear(sf::RenderWindow* window, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
        if (window) {
            window->clear(sf::Color(r, g, b, a));
        }
    }

    bool sfml_window_poll_event(sf::RenderWindow* window, sf::Event* event) {
        return window && event && window->pollEvent(*event);
    }

    void sfml_window_draw_sprite(sf::RenderWindow* window, const sf::Sprite* sprite) {
        if (window && sprite) {
            window->draw(*sprite);
        }
    }

    void sfml_window_display(sf::RenderWindow* window) {
        if (window) {
            window->display();
        }
    }

    sf::Texture* sfml_load_texture(const char* path) {
        sf::Texture* texture = new sf::Texture();
        if (!texture->loadFromFile(path)) {
            delete texture;
            return nullptr;
        }
        return texture;
    }

    void sfml_destroy_texture(sf::Texture* texture) {
        delete texture;
    }
}