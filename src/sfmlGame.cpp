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
    return true;
}

SFMLGame::SFMLGame() : _window(nullptr), _width(800), _height(600), _michaelMode(0),
    _snakeUpDownTexture(nullptr), _snake2UpDownTexture(nullptr),
    _snakeLeftRightTexture(nullptr), _snake2LeftRightTexture(nullptr),
    _snakeTurnRightTexture(nullptr), _snake2TurnRightTexture(nullptr),
    _snakeTurnLeftTexture(nullptr), _snake2TurnLeftTexture(nullptr),
    _foodTexture(nullptr), _backgroundTexture1(nullptr),
    _backgroundTexture2(nullptr), _wallTexture(nullptr),
    _snakeHeadTexture(nullptr), _snake2HeadTexture(nullptr),
    _snakeHeadTurnLeftTexture(nullptr), _snake2HeadTurnLeftTexture(nullptr),
    _snakeHeadTurnRightTexture(nullptr), _snake2HeadTurnRightTexture(nullptr),
    _snakeTailTexture(nullptr), _snake2TailTexture(nullptr),
    _michaelModeTexture(nullptr), _lastMoveTime(), _lastMoveTime2(), _prevSnakeBody(), _prevSnakeBody2() {
}

SFMLGame::SFMLGame(const SFMLGame& other) : _window(other._window), _width(other._width), _height(other._height), _michaelMode(other._michaelMode),
    _snakeUpDownTexture(other._snakeUpDownTexture), _snakeLeftRightTexture(other._snakeLeftRightTexture),
    _snakeTurnRightTexture(other._snakeTurnRightTexture), _snakeTurnLeftTexture(other._snakeTurnLeftTexture),
    _foodTexture(other._foodTexture), _backgroundTexture1(other._backgroundTexture1),
    _backgroundTexture2(other._backgroundTexture2), _wallTexture(other._wallTexture),
    _snakeHeadTexture(other._snakeHeadTexture), _snakeHeadTurnLeftTexture(other._snakeHeadTurnLeftTexture),
    _snakeHeadTurnRightTexture(other._snakeHeadTurnRightTexture), _snakeTailTexture(other._snakeTailTexture), _michaelModeTexture(other._michaelModeTexture),
    _lastMoveTime(other._lastMoveTime), _lastMoveTime2(other._lastMoveTime2), _prevSnakeBody(other._prevSnakeBody), _prevSnakeBody2(other._prevSnakeBody2) {
}

SFMLGame& SFMLGame::operator=(const SFMLGame& other) {
    if (this != &other) {
        _window = other._window;
        _width = other._width;
        _height = other._height;
        _michaelMode = other._michaelMode;
        _snakeUpDownTexture = other._snakeUpDownTexture;
        _snake2UpDownTexture = other._snake2UpDownTexture;
        _snakeLeftRightTexture = other._snakeLeftRightTexture;
        _snake2LeftRightTexture = other._snake2LeftRightTexture;
        _snakeTurnRightTexture = other._snakeTurnRightTexture;
        _snake2TurnRightTexture = other._snake2TurnRightTexture;
        _snakeTurnLeftTexture = other._snakeTurnLeftTexture;
        _snake2TurnLeftTexture = other._snake2TurnLeftTexture;
        _foodTexture = other._foodTexture;
        _backgroundTexture1 = other._backgroundTexture1;
        _backgroundTexture2 = other._backgroundTexture2;
        _wallTexture = other._wallTexture;
        _snakeHeadTexture = other._snakeHeadTexture;
        _snake2HeadTexture = other._snake2HeadTexture;
        _snakeHeadTurnLeftTexture = other._snakeHeadTurnLeftTexture;
        _snakeHeadTurnRightTexture = other._snakeHeadTurnRightTexture;
        _snakeTailTexture = other._snakeTailTexture;
        _snake2TailTexture = other._snake2TailTexture;
        _michaelModeTexture = other._michaelModeTexture;
        _lastMoveTime = other._lastMoveTime;
        _lastMoveTime2 = other._lastMoveTime2;
        _prevSnakeBody = other._prevSnakeBody;
        _prevSnakeBody2 = other._prevSnakeBody2;
    }
    return *this;
}

SFMLGame::SFMLGame(int w, int h, bool michaelMode) : _window(nullptr), _width(w), _height(h), _michaelMode(michaelMode),
    _snakeUpDownTexture(nullptr), _snake2UpDownTexture(nullptr),
    _snakeLeftRightTexture(nullptr), _snake2LeftRightTexture(nullptr),
    _snakeTurnRightTexture(nullptr), _snake2TurnRightTexture(nullptr),
    _snakeTurnLeftTexture(nullptr), _snake2TurnLeftTexture(nullptr),
    _foodTexture(nullptr), _backgroundTexture1(nullptr),
    _backgroundTexture2(nullptr), _wallTexture(nullptr),
    _snakeHeadTexture(nullptr), _snake2HeadTexture(nullptr),
    _snakeHeadTurnLeftTexture(nullptr), _snake2HeadTurnLeftTexture(nullptr),
    _snakeHeadTurnRightTexture(nullptr), _snake2HeadTurnRightTexture(nullptr),
    _snakeTailTexture(nullptr), _snake2TailTexture(nullptr),
    _michaelModeTexture(nullptr), _lastMoveTime(), _lastMoveTime2(), _prevSnakeBody(), _prevSnakeBody2() {
    if (!load_sfml_symbols()) {
        return;
    }

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

    _snakeHeadTexture = SFML_LoadTexture_ptr("textureSFML/snake_head_up.png");
    if (!_snakeHeadTexture) {
        std::cerr << "[SFML] Error: Unable to load snake_head_up.png" << std::endl;
    }

    _snakeHeadTurnLeftTexture = SFML_LoadTexture_ptr("textureSFML/snake_head_turn_left.png");
    if (!_snakeHeadTurnLeftTexture) {
        std::cerr << "[SFML] Error: Unable to load snake_head_turn_left.png" << std::endl;
    }

    _snakeHeadTurnRightTexture = SFML_LoadTexture_ptr("textureSFML/snake_head_turn_right.png");
    if (!_snakeHeadTurnRightTexture) {
        std::cerr << "[SFML] Error: Unable to load snake_head_turn_right.png" << std::endl;
    }

    _snakeTailTexture = SFML_LoadTexture_ptr("textureSFML/snake_tail_up.png");
    if (!_snakeTailTexture) {
        std::cerr << "[SFML] Error: Unable to load snake_tail_up.png" << std::endl;
    }

    _snake2HeadTexture = SFML_LoadTexture_ptr("textureSFML/player_2_head_up.png");
    if (!_snake2HeadTexture) {
        std::cerr << "[SFML] Error: Unable to load player_2_head_up.png" << std::endl;
    }

    _snake2HeadTurnLeftTexture = SFML_LoadTexture_ptr("textureSFML/player_2_head_turn_left.png");
    if (!_snake2HeadTurnLeftTexture) {
        std::cerr << "[SFML] Error: Unable to load player_2_head_turn_left.png" << std::endl;
    }

    _snake2HeadTurnRightTexture = SFML_LoadTexture_ptr("textureSFML/player_2_head_turn_right.png");
    if (!_snake2HeadTurnRightTexture) {
        std::cerr << "[SFML] Error: Unable to load player_2_head_turn_right.png" << std::endl;
    }

    _snake2TailTexture = SFML_LoadTexture_ptr("textureSFML/player_2_tail_up.png");
    if (!_snake2TailTexture) {
        std::cerr << "[SFML] Error: Unable to load player_2_tail_up.png" << std::endl;
    }

    _snake2UpDownTexture = SFML_LoadTexture_ptr("textureSFML/player_2_up_down.png");
    if (!_snake2UpDownTexture) {
        std::cerr << "[SFML] Error: Unable to load player_2_up_down.png" << std::endl;
    }

    _snake2LeftRightTexture = SFML_LoadTexture_ptr("textureSFML/player_2_body_left.png");
    if (!_snake2LeftRightTexture) {
        std::cerr << "[SFML] Error: Unable to load player_2_body_left.png" << std::endl;
    }

    _snake2TurnRightTexture = SFML_LoadTexture_ptr("textureSFML/player_2_turn_right.png");
    if (!_snake2TurnRightTexture) {
        std::cerr << "[SFML] Error: Unable to load player_2_turn_right.png" << std::endl;
    }

    _snake2TurnLeftTexture = SFML_LoadTexture_ptr("textureSFML/player_2_turn_left.png");
    if (!_snake2TurnLeftTexture) {
        std::cerr << "[SFML] Error: Unable to load player_2_turn_left.png" << std::endl;
    }

    _michaelModeTexture = SFML_LoadTexture_ptr("textureSFML/projet_michael_youn.png");
    if (!_michaelModeTexture) {
        std::cerr << "[SFML] Error: Unable to load projet_michael_youn.png" << std::endl;
    }

    _wallTexture = SFML_LoadTexture_ptr("textureSFML/wall_sfml.png");
    if (!_wallTexture) {
        std::cerr << "[SFML] Error: Unable to load wall_sfml.png" << std::endl;
    }
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
    if (_snakeHeadTurnLeftTexture) SFML_DestroyTexture_ptr(_snakeHeadTurnLeftTexture);
    if (_snakeHeadTurnRightTexture) SFML_DestroyTexture_ptr(_snakeHeadTurnRightTexture);
    if (_snakeTailTexture) SFML_DestroyTexture_ptr(_snakeTailTexture);
    if (_snake2HeadTexture) SFML_DestroyTexture_ptr(_snake2HeadTexture);
    if (_snake2HeadTurnLeftTexture) SFML_DestroyTexture_ptr(_snake2HeadTurnLeftTexture);
    if (_snake2HeadTurnRightTexture) SFML_DestroyTexture_ptr(_snake2HeadTurnRightTexture);
    if (_snake2TailTexture) SFML_DestroyTexture_ptr(_snake2TailTexture);
    if (_snake2UpDownTexture) SFML_DestroyTexture_ptr(_snake2UpDownTexture);
    if (_snake2LeftRightTexture) SFML_DestroyTexture_ptr(_snake2LeftRightTexture);
    if (_snake2TurnRightTexture) SFML_DestroyTexture_ptr(_snake2TurnRightTexture);
    if (_snake2TurnLeftTexture) SFML_DestroyTexture_ptr(_snake2TurnLeftTexture);
    if (_michaelModeTexture) SFML_DestroyTexture_ptr(_michaelModeTexture);
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

void SFMLGame::display_good_part(int currentDirection, const std::vector<std::pair<int, int>>& snakeBody, bool modeMichael, bool multi,
    std::chrono::high_resolution_clock::time_point& lastMoveTime,
    std::vector<std::pair<int, int>>& prevSnakeBody) {
    const float offsetX = modeMichael ? 640.0f : 0.0f;
    const float offsetY = modeMichael ? 350.0f : 0.0f;

    sf::Texture* headTexture = multi ? _snake2HeadTexture : _snakeHeadTexture;
    sf::Texture* headTurnLeftTexture = multi ? _snake2HeadTurnLeftTexture : _snakeHeadTurnLeftTexture;
    sf::Texture* headTurnRightTexture = multi ? _snake2HeadTurnRightTexture : _snakeHeadTurnRightTexture;
    sf::Texture* tailTexture = multi ? _snake2TailTexture : _snakeTailTexture;
    sf::Texture* upDownTexture = multi ? _snake2UpDownTexture : _snakeUpDownTexture;
    sf::Texture* turnLeftTexture = multi ? _snake2TurnLeftTexture : _snakeTurnLeftTexture;

    if (prevSnakeBody.empty()) {
        prevSnakeBody = snakeBody;
        lastMoveTime = std::chrono::high_resolution_clock::now();
    }

    auto now = std::chrono::high_resolution_clock::now();
    if (prevSnakeBody != snakeBody) {
        lastMoveTime = now;
        prevSnakeBody = snakeBody;
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMoveTime).count();
    float progress = std::min(1.0f, elapsed / 150.0f);

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
        float centerX = 0.0f;
        float centerY = 0.0f;

        if (i < prevSnakeBody.size()) {
            float oldX = (prevSnakeBody[i].first * 32.0f + 16.0f) + offsetX;
            float oldY = (prevSnakeBody[i].second * 32.0f + 16.0f) + offsetY;
            float newX = (snakeBody[i].first * 32.0f + 16.0f) + offsetX;
            float newY = (snakeBody[i].second * 32.0f + 16.0f) + offsetY;
            centerX = oldX + (newX - oldX) * progress;
            centerY = oldY + (newY - oldY) * progress;
        } else {
            centerX = (snakeBody[i].first * 32.0f + 16.0f) + offsetX;
            centerY = (snakeBody[i].second * 32.0f + 16.0f) + offsetY;
        }

        if (i == 0) {
            int fromX = snakeBody[1].first - snakeBody[0].first;
            int fromY = snakeBody[1].second - snakeBody[0].second;

            sf::Sprite headSprite(*headTexture);
            headSprite.setPosition(centerX, centerY);
            headSprite.setOrigin(
                16.0f + (0.25f * getDir(currentDirection, 'x')),
                16.0f + (0.25f * getDir(currentDirection, 'y'))
            );

            if ((currentDirection == UP    && fromY == 1)  ||
                (currentDirection == DOWN  && fromY == -1) ||
                (currentDirection == LEFT  && fromX == 1)  ||
                (currentDirection == RIGHT && fromX == -1))
            {
                headSprite.setRotation(directionAngle(currentDirection));
                SFML_WindowDrawSprite_ptr(_window, &headSprite);
            }

            else if (currentDirection == UP && fromX == -1)
            {
                sf::Sprite turn(*headTurnLeftTexture);
                turn.setPosition(centerX, centerY);
                turn.setOrigin(16.0f, 16.0f);
                turn.setRotation(0.0f);
                SFML_WindowDrawSprite_ptr(_window, &turn);
            }

            else if (currentDirection == UP && fromX == 1)
            {
                sf::Sprite turn(*headTurnRightTexture);
                turn.setPosition(centerX, centerY);
                turn.setOrigin(16.0f, 16.0f);
                turn.setRotation(0.0f);
                SFML_WindowDrawSprite_ptr(_window, &turn);
            }

            else if (currentDirection == DOWN && fromX == -1)
            {
                sf::Sprite turn(*headTurnRightTexture);
                turn.setPosition(centerX, centerY);
                turn.setOrigin(16.0f, 16.0f);
                turn.setRotation(180.0f);
                SFML_WindowDrawSprite_ptr(_window, &turn);
            }

            else if (currentDirection == DOWN && fromX == 1)
            {
                sf::Sprite turn(*headTurnLeftTexture);
                turn.setPosition(centerX, centerY);
                turn.setOrigin(16.0f, 16.0f);
                turn.setRotation(180.0f);
                SFML_WindowDrawSprite_ptr(_window, &turn);
            }

            else if (currentDirection == LEFT && fromY == -1)
            {
                sf::Sprite turn(*headTurnRightTexture);
                turn.setPosition(centerX, centerY);
                turn.setOrigin(16.0f, 16.0f);
                turn.setRotation(270.0f);
                SFML_WindowDrawSprite_ptr(_window, &turn);
            }

            else if (currentDirection == LEFT && fromY == 1)
            {
                sf::Sprite turn(*headTurnLeftTexture);
                turn.setPosition(centerX, centerY);
                turn.setOrigin(16.0f, 16.0f);
                turn.setRotation(270.0f);
                SFML_WindowDrawSprite_ptr(_window, &turn);
            }

            else if (currentDirection == RIGHT && fromY == -1)
            {
                sf::Sprite turn(*headTurnLeftTexture);
                turn.setPosition(centerX, centerY);
                turn.setOrigin(16.0f, 16.0f);
                turn.setRotation(90.0f);
                SFML_WindowDrawSprite_ptr(_window, &turn);
            }

            else if (currentDirection == RIGHT && fromY == 1)
            {
                sf::Sprite turn(*headTurnRightTexture);
                turn.setPosition(centerX, centerY);
                turn.setOrigin(16.0f, 16.0f);
                turn.setRotation(90.0f);
                SFML_WindowDrawSprite_ptr(_window, &turn);
            }
        } else if (i == snakeBody.size() - 1) {
            auto prev = snakeBody[i - 1];
            auto tail = snakeBody[i];
            int dx = tail.first - prev.first;
            int dy = tail.second - prev.second;

            sf::Sprite tailSprite(*tailTexture);
            tailSprite.setPosition(centerX, centerY);
            tailSprite.setOrigin(16.0f, 16.0f);
            tailSprite.setRotation(segmentAngleFromDelta(-dx, -dy));
            SFML_WindowDrawSprite_ptr(_window, &tailSprite);
        } else {
            auto prev = snakeBody[i - 1];
            auto curr = snakeBody[i];
            auto next = snakeBody[i + 1];

            int dx1 = curr.first - prev.first;
            int dy1 = curr.second - prev.second;
            int dx2 = next.first - curr.first;
            int dy2 = next.second - curr.second;

            if ((dx1 == 0 && dx2 == 0) || (dy1 == 0 && dy2 == 0)) {
                sf::Sprite bodySprite(*upDownTexture);
                bodySprite.setPosition(centerX, centerY);
                bodySprite.setOrigin(16.0f, 16.0f);
                bodySprite.setRotation(segmentAngleFromDelta(-dx1, -dy1));
                SFML_WindowDrawSprite_ptr(_window, &bodySprite);
            } else {
                float angle = 0.0f;

                if (dx1 == 0 && dy1 == -1 && dx2 == -1 && dy2 == 0) angle = 270.0f;
                else if (dx1 == -1 && dy1 == 0 && dx2 == 0 && dy2 == 1) angle = 180.0f;
                else if (dx1 == 0 && dy1 == 1 && dx2 == 1 && dy2 == 0) angle = 90.0f;
                else if (dx1 == 1 && dy1 == 0 && dx2 == 0 && dy2 == -1) angle = 0.0f;
                else if (dx2 == 0 && dy2 == -1 && dx1 == -1 && dy1 == 0) angle = 90.0f;
                else if (dx2 == -1 && dy2 == 0 && dx1 == 0 && dy1 == 1) angle = 0.0f;
                else if (dx2 == 1 && dy2 == 0 && dx1 == 0 && dy1 == -1) angle = 180.0f;
                else if (dx2 == 0 && dy2 == 1 && dx1 == 1 && dy1 == 0) angle = 270.0f;

                sf::Sprite turnSprite(*turnLeftTexture);
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
                timeval now;
                gettimeofday(&now, nullptr);
                long elapsed = (now.tv_sec - game.getSpawnApple().tv_sec) * 1000 + (now.tv_usec - game.getSpawnApple().tv_usec) / 1000;
                if (game.getAppelDespawned() && elapsed > (game.getWidth() + game.getHeight()) * 16) {
                    continue;
                } else {
                    sf::Sprite foodSprite(*_foodTexture);
                    foodSprite.setPosition(((i * 32.0f) + 16.0f) + offsetX, ((j * 32.0f) + 16.0f) + offsetY);
                    foodSprite.setOrigin(16.0f, 16.0f);
                    SFML_WindowDrawSprite_ptr(_window, &foodSprite);
                }
            } else if (cell == WALL) {
                sf::Sprite wallSprite(*_wallTexture);
                wallSprite.setPosition((i * 32.0f) + 16.0f + offsetX, ((j * 32.0f) + 16.0f) + offsetY);
                wallSprite.setOrigin(16.0f, 16.0f);
                SFML_WindowDrawSprite_ptr(_window, &wallSprite);
            }
        }
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

    if (_snakeUpDownTexture && _snakeLeftRightTexture && _snakeTurnRightTexture && _snakeTurnLeftTexture) {
        display_good_part(game.getCurrentDirection(), game.getSnakeBody(), game.getMichaelMode(), false, _lastMoveTime, _prevSnakeBody);
        if (game.getNbPlayer() >= 2 &&
            _snake2UpDownTexture && _snake2LeftRightTexture &&
            _snake2TurnRightTexture && _snake2TurnLeftTexture &&
            _snake2HeadTexture && _snake2TailTexture) {
            display_good_part(game.getCurrentDirection2(), game.getSnakeBody2(), game.getMichaelMode(), true, _lastMoveTime2, _prevSnakeBody2);
        }
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
            switch (event.key.code) {
                case sf::Keyboard::W:
                    return P2_UP;
                case sf::Keyboard::S:
                    return P2_DOWN;
                case sf::Keyboard::A:
                    return P2_LEFT;
                case sf::Keyboard::D:
                    return P2_RIGHT;
                case sf::Keyboard::Up:
                    return UP;
                case sf::Keyboard::Down:
                    return DOWN;
                case sf::Keyboard::Left:
                    return LEFT;
                case sf::Keyboard::Right:
                    return RIGHT;
                case sf::Keyboard::Escape:
                    std::cerr << "[INPUT] ESCAPE detected -> returning -1" << std::endl;
                    return -1;
                case sf::Keyboard::Num1:
                    currentLibrary = SDL3;
                    return 10;
                case sf::Keyboard::Num2:
                    currentLibrary = SFML;
                    return 20;
                case sf::Keyboard::Num3:
                    currentLibrary = GL;
                    return 30;
                case sf::Keyboard::P:
                    return 1000;
                default:
                    break;
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