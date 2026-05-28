#include "../includes/Sdl3Game.hpp"

// SDL3 constants
#define SDL_INIT_VIDEO 0x00000020

// SDL3 event types
#define SDL_EVENT_QUIT 0x100
#define SDL_EVENT_KEY_DOWN 0x300
#define SDL_EVENT_KEY_UP 0x301

// SDL3 key codes (scancode based in SDL3)
#define SDL_SCANCODE_LEFT 0x50
#define SDL_SCANCODE_RIGHT 0x4F
#define SDL_SCANCODE_UP 0x52
#define SDL_SCANCODE_DOWN 0x51
#define SDL_SCANCODE_ESCAPE 0x29
#define SDL_SCANCODE_1 0x1E
#define SDL_SCANCODE_2 0x1F
#define SDL_SCANCODE_3 0x20

#define SDL_BLENDMODE_BLEND 0x00000001u
#define SDL_BLENDMODE_BLEND_PREMULTIPLIED 0x00000010u

typedef int (*PFNSDLINITPROC)(int);
typedef int (*PFNSDLQUITPROC)(void);
typedef SDL_Window* (*PFNSDLCREATEWINDOWPROC)(const char*, int, int, unsigned int);
typedef SDL_Renderer* (*PFNSDLCREATERENDERERPROC)(SDL_Window*, const char*);
typedef void (*PFNSDLDESTROYWINDOWPROC)(SDL_Window*);
typedef void (*PFNSDLDESTROYRENDERERPROC)(SDL_Renderer*);
typedef const char* (*PFNSDLGETERRORPROC)(void);
typedef void (*PFNSDLDESTROYEXTUREPROC)(SDL_Texture*);
typedef bool (*PFNSDLSETRENDERDRAWCOLORPROC)(SDL_Renderer*, unsigned char, unsigned char, unsigned char, unsigned char);
typedef bool (*PFNSDLRENDERCLEARPROC)(SDL_Renderer*);
typedef bool (*PFNSDLRENDERFILLRECTPROC)(SDL_Renderer*, const SDL_FRect*);
typedef bool (*PFNSDLRENDERPRESENTPROC)(SDL_Renderer*);
typedef int (*PFNSDLPOLLEVENTPROC)(SDL_Event*);
typedef SDL_Surface* (*PFNSDLLOADBMPPROC)(const char*);
typedef void (*PFNSDLDESTROYPROC)(SDL_Surface*);
typedef SDL_Texture* (*PFNSDLCREATETEXTUREFROMSURFACEPROC)(SDL_Renderer*, SDL_Surface*);
typedef bool (*PFNSDLRENDERTEXTUREPROC)(SDL_Renderer*, SDL_Texture*, const SDL_FRect*, const SDL_FRect*);
typedef bool (*PFNSDLRENDERTEXTUREROTATEDPROC)(SDL_Renderer*, SDL_Texture*, const SDL_FRect*, const SDL_FRect*, double, const SDL_FPoint*, SDL_FlipMode);
typedef bool (*PFNSDLSETWINDOWINPUTFOCUSPROC)(SDL_Window*);
typedef bool (*PFNSDLSETSURFACECOLORKEYPROC)(SDL_Surface*, bool, unsigned int);
typedef bool (*PFNSDLSETTEXTUREBLENDMODEPROC)(SDL_Texture*, unsigned int);

static PFNSDLINITPROC SDL_Init_ptr = nullptr;
static PFNSDLQUITPROC SDL_Quit_ptr = nullptr;
static PFNSDLCREATEWINDOWPROC SDL_CreateWindow_ptr = nullptr;
static PFNSDLCREATERENDERERPROC SDL_CreateRenderer_ptr = nullptr;
static PFNSDLDESTROYWINDOWPROC SDL_DestroyWindow_ptr = nullptr;
static PFNSDLDESTROYRENDERERPROC SDL_DestroyRenderer_ptr = nullptr;
static PFNSDLGETERRORPROC SDL_GetError_ptr = nullptr;
static PFNSDLDESTROYEXTUREPROC SDL_DestroyTexture_ptr = nullptr;
static PFNSDLSETRENDERDRAWCOLORPROC SDL_SetRenderDrawColor_ptr = nullptr;
static PFNSDLRENDERCLEARPROC SDL_RenderClear_ptr = nullptr;
static PFNSDLRENDERFILLRECTPROC SDL_RenderFillRect_ptr = nullptr;
static PFNSDLRENDERPRESENTPROC SDL_RenderPresent_ptr = nullptr;
static PFNSDLPOLLEVENTPROC SDL_PollEvent_ptr = nullptr;
static PFNSDLLOADBMPPROC SDL_LoadBMP_ptr = nullptr;
static PFNSDLDESTROYPROC SDL_DestroySurface_ptr = nullptr;
static PFNSDLCREATETEXTUREFROMSURFACEPROC SDL_CreateTextureFromSurface_ptr = nullptr;
static PFNSDLRENDERTEXTUREPROC SDL_RenderTexture_ptr = nullptr;
static PFNSDLRENDERTEXTUREROTATEDPROC SDL_RenderTextureRotated_ptr = nullptr;
static PFNSDLSETWINDOWINPUTFOCUSPROC SDL_SetWindowInputFocus_ptr = nullptr;
static PFNSDLSETSURFACECOLORKEYPROC SDL_SetSurfaceColorKey_ptr = nullptr;
static PFNSDLSETTEXTUREBLENDMODEPROC SDL_SetTextureBlendMode_ptr = nullptr;
static void* sdl_handle = nullptr;
static bool initialized = false;

static bool load_sdl3_symbols() {
    if (initialized) return true;
    
    sdl_handle = dlopen("./sdl3/build/libSDL3.so", RTLD_NOW | RTLD_GLOBAL);
    if (!sdl_handle) {
        std::cerr << "[SDL3] Error: Unable to load libSDL3 - " << dlerror() << std::endl;
        return false;
    }
        
    // Load function pointers from SDL3
    SDL_Init_ptr = (PFNSDLINITPROC)dlsym(sdl_handle, "SDL_Init");
    SDL_Quit_ptr = (PFNSDLQUITPROC)dlsym(sdl_handle, "SDL_Quit");
    SDL_CreateWindow_ptr = (PFNSDLCREATEWINDOWPROC)dlsym(sdl_handle, "SDL_CreateWindow");
    SDL_CreateRenderer_ptr = (PFNSDLCREATERENDERERPROC)dlsym(sdl_handle, "SDL_CreateRenderer");
    SDL_DestroyWindow_ptr = (PFNSDLDESTROYWINDOWPROC)dlsym(sdl_handle, "SDL_DestroyWindow");
    SDL_DestroyRenderer_ptr = (PFNSDLDESTROYRENDERERPROC)dlsym(sdl_handle, "SDL_DestroyRenderer");
    SDL_GetError_ptr = (PFNSDLGETERRORPROC)dlsym(sdl_handle, "SDL_GetError");
    SDL_SetRenderDrawColor_ptr = (PFNSDLSETRENDERDRAWCOLORPROC)dlsym(sdl_handle, "SDL_SetRenderDrawColor");
    SDL_RenderClear_ptr = (PFNSDLRENDERCLEARPROC)dlsym(sdl_handle, "SDL_RenderClear");
    SDL_RenderFillRect_ptr = (PFNSDLRENDERFILLRECTPROC)dlsym(sdl_handle, "SDL_RenderFillRect");
    SDL_RenderPresent_ptr = (PFNSDLRENDERPRESENTPROC)dlsym(sdl_handle, "SDL_RenderPresent");
    SDL_PollEvent_ptr = (PFNSDLPOLLEVENTPROC)dlsym(sdl_handle, "SDL_PollEvent");
    SDL_LoadBMP_ptr = (PFNSDLLOADBMPPROC)dlsym(sdl_handle, "SDL_LoadBMP");
    
    // Load SDL3 image and texture functions
    SDL_DestroySurface_ptr = (PFNSDLDESTROYPROC)dlsym(sdl_handle, "SDL_DestroySurface");
    SDL_CreateTextureFromSurface_ptr = (PFNSDLCREATETEXTUREFROMSURFACEPROC)dlsym(sdl_handle, "SDL_CreateTextureFromSurface");
    SDL_RenderTexture_ptr = (PFNSDLRENDERTEXTUREPROC)dlsym(sdl_handle, "SDL_RenderTexture");
    SDL_RenderTextureRotated_ptr = (PFNSDLRENDERTEXTUREROTATEDPROC)dlsym(sdl_handle, "SDL_RenderTextureRotated");
    SDL_SetWindowInputFocus_ptr = (PFNSDLSETWINDOWINPUTFOCUSPROC)dlsym(sdl_handle, "SDL_SetWindowInputFocus");
    SDL_DestroyTexture_ptr = (PFNSDLDESTROYEXTUREPROC)dlsym(sdl_handle, "SDL_DestroyTexture");
    SDL_SetSurfaceColorKey_ptr = (PFNSDLSETSURFACECOLORKEYPROC)dlsym(sdl_handle, "SDL_SetSurfaceColorKey");
    SDL_SetTextureBlendMode_ptr = (PFNSDLSETTEXTUREBLENDMODEPROC)dlsym(sdl_handle, "SDL_SetTextureBlendMode");
    
    if (!SDL_Init_ptr || !SDL_CreateWindow_ptr || !SDL_CreateRenderer_ptr || 
        !SDL_DestroyWindow_ptr || !SDL_DestroyRenderer_ptr || !SDL_GetError_ptr ||
        !SDL_SetRenderDrawColor_ptr || !SDL_RenderClear_ptr ||
        !SDL_RenderFillRect_ptr || !SDL_RenderPresent_ptr || !SDL_PollEvent_ptr ||
        !SDL_LoadBMP_ptr || !SDL_DestroySurface_ptr || !SDL_CreateTextureFromSurface_ptr ||
        !SDL_RenderTexture_ptr || !SDL_RenderTextureRotated_ptr || !SDL_SetSurfaceColorKey_ptr || !SDL_SetTextureBlendMode_ptr) {
        std::cerr << "[SDL3] Error: Unable to load all SDL3 symbols" << std::endl;
        return false;
    }
    
    initialized = true;
    return true;
}

SDL3Game::SDL3Game() : _window(nullptr), _renderer(nullptr), _width(0), _height(0),
    _snakeUpDownTexture(nullptr), _snakeLeftRightTexture(nullptr), _snakeTurnRightTexture(nullptr),
    _snakeTurnLeftTexture(nullptr), _foodTexture(nullptr), _backgroundTexture(nullptr), _wallTexture(nullptr), _michaelModeTexture(nullptr) {
}

SDL3Game::SDL3Game(const SDL3Game& other) : _window(other._window), _renderer(other._renderer), _width(other._width), _height(other._height),
    _snakeUpDownTexture(other._snakeUpDownTexture), _snakeLeftRightTexture(other._snakeLeftRightTexture),
    _snakeTurnRightTexture(other._snakeTurnRightTexture), _snakeTurnLeftTexture(other._snakeTurnLeftTexture),
    _foodTexture(other._foodTexture), _backgroundTexture(other._backgroundTexture), _wallTexture(other._wallTexture), _michaelModeTexture(other._michaelModeTexture) {
}

SDL3Game& SDL3Game::operator=(const SDL3Game& other) {
    if (this != &other) {
        _window = other._window;
        _renderer = other._renderer;
        _width = other._width;
        _height = other._height;
        _snakeUpDownTexture = other._snakeUpDownTexture;
        _snakeLeftRightTexture = other._snakeLeftRightTexture;
        _snakeTurnRightTexture = other._snakeTurnRightTexture;
        _snakeTurnLeftTexture = other._snakeTurnLeftTexture;
        _foodTexture = other._foodTexture;
        _backgroundTexture = other._backgroundTexture;
        _wallTexture = other._wallTexture;
        _michaelModeTexture = other._michaelModeTexture;
    }
    return *this;
}

SDL3Game::SDL3Game(int w, int h, bool michaelMode) : _window(nullptr), _renderer(nullptr), _width(w), _height(h),
    _snakeUpDownTexture(nullptr), _snakeLeftRightTexture(nullptr), _snakeTurnRightTexture(nullptr),
    _snakeTurnLeftTexture(nullptr), _foodTexture(nullptr), _backgroundTexture(nullptr), _wallTexture(nullptr) {
    
    if (!load_sdl3_symbols()) {
        return;
    }
    
    if (SDL_Init_ptr(SDL_INIT_VIDEO) < 0) {
        std::cerr << "[SDL3] SDL_Init failed: " << SDL_GetError_ptr() << std::endl;
        return;
    }
    
    int window_width = 0;
    int window_height = 0;
    if (michaelMode) {
        window_width = 1400;
        window_height = 1800;
    } else {
        // Create window with appropriate size
        window_width = (w * 32 > 400) ? w * 32 : 400;
        window_height = (h * 32 > 400) ? h * 32 : 400;
    }
    
    _window = SDL_CreateWindow_ptr("Nibbler - SDL3", window_width, window_height, 0);  // flags=0 for normal window
    _renderer = SDL_CreateRenderer_ptr(_window, nullptr);

    if (!_window || !_renderer) {
        std::cerr << "[SDL3] Window/Renderer creation failed: " << SDL_GetError_ptr() << std::endl;
        SDL_Quit_ptr();
        return;
    }
    
    // Capture window focus to avoid system shortcuts
    if (SDL_SetWindowInputFocus_ptr) {
        SDL_SetWindowInputFocus_ptr(_window);
    }
    
    // Load BMP textures from textureSDL3/ directory
    SDL_Surface* surface = nullptr;
    
    surface = SDL_LoadBMP_ptr("textureSDL3/snake_up_down.bmp");
    if (surface) {
        SDL_SetSurfaceColorKey_ptr(surface, true, 0x000000);
        _snakeUpDownTexture = SDL_CreateTextureFromSurface_ptr(_renderer, surface);
        if (_snakeUpDownTexture) SDL_SetTextureBlendMode_ptr(_snakeUpDownTexture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
        SDL_DestroySurface_ptr(surface);
    }
    
    surface = SDL_LoadBMP_ptr("textureSDL3/snake_right_left.bmp");
    if (surface) {
        SDL_SetSurfaceColorKey_ptr(surface, true, 0x000000);
        _snakeLeftRightTexture = SDL_CreateTextureFromSurface_ptr(_renderer, surface);
        if (_snakeLeftRightTexture) SDL_SetTextureBlendMode_ptr(_snakeLeftRightTexture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
        SDL_DestroySurface_ptr(surface);
    }
    
    surface = SDL_LoadBMP_ptr("textureSDL3/snake_turn_right.bmp");
    if (surface) {
        SDL_SetSurfaceColorKey_ptr(surface, true, 0x000000);
        _snakeTurnRightTexture = SDL_CreateTextureFromSurface_ptr(_renderer, surface);
        if (_snakeTurnRightTexture) SDL_SetTextureBlendMode_ptr(_snakeTurnRightTexture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
        SDL_DestroySurface_ptr(surface);
    }
    
    surface = SDL_LoadBMP_ptr("textureSDL3/snake_turn_left.bmp");
    if (surface) {
        SDL_SetSurfaceColorKey_ptr(surface, true, 0x000000);
        _snakeTurnLeftTexture = SDL_CreateTextureFromSurface_ptr(_renderer, surface);
        if (_snakeTurnLeftTexture) SDL_SetTextureBlendMode_ptr(_snakeTurnLeftTexture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
        SDL_DestroySurface_ptr(surface);
    }
    
    surface = SDL_LoadBMP_ptr("textureSDL3/apple.bmp");
    if (surface) {
        SDL_SetSurfaceColorKey_ptr(surface, true, 0x000000);
        _foodTexture = SDL_CreateTextureFromSurface_ptr(_renderer, surface);
        if (_foodTexture) SDL_SetTextureBlendMode_ptr(_foodTexture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
        SDL_DestroySurface_ptr(surface);
    }
    
    surface = SDL_LoadBMP_ptr("textureSDL3/bg.bmp");
    if (surface) {
        SDL_SetSurfaceColorKey_ptr(surface, true, 0x000000);
        _backgroundTexture = SDL_CreateTextureFromSurface_ptr(_renderer, surface);
        if (_backgroundTexture) SDL_SetTextureBlendMode_ptr(_backgroundTexture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
        SDL_DestroySurface_ptr(surface);
    }
    
    surface = SDL_LoadBMP_ptr("textureSDL3/wall.bmp");
    if (surface) {
        SDL_SetSurfaceColorKey_ptr(surface, true, 0x000000);
        _wallTexture = SDL_CreateTextureFromSurface_ptr(_renderer, surface);
        if (_wallTexture) SDL_SetTextureBlendMode_ptr(_wallTexture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
        SDL_DestroySurface_ptr(surface);
    }

    surface = SDL_LoadBMP_ptr("textureSDL3/projet_michael_youn.bmp");
    if (surface) {
        SDL_SetSurfaceColorKey_ptr(surface, true, 0x000000);
        _michaelModeTexture = SDL_CreateTextureFromSurface_ptr(_renderer, surface);
        if (_michaelModeTexture) SDL_SetTextureBlendMode_ptr(_michaelModeTexture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
        SDL_DestroySurface_ptr(surface);
    }
}

SDL3Game::~SDL3Game() {
    if (_snakeUpDownTexture) SDL_DestroyTexture_ptr(_snakeUpDownTexture);
    if (_snakeLeftRightTexture) SDL_DestroyTexture_ptr(_snakeLeftRightTexture);
    if (_snakeTurnRightTexture) SDL_DestroyTexture_ptr(_snakeTurnRightTexture);
    if (_snakeTurnLeftTexture) SDL_DestroyTexture_ptr(_snakeTurnLeftTexture);
    if (_foodTexture) SDL_DestroyTexture_ptr(_foodTexture);
    if (_backgroundTexture) SDL_DestroyTexture_ptr(_backgroundTexture);
    if (_wallTexture) SDL_DestroyTexture_ptr(_wallTexture);
    if (_renderer) SDL_DestroyRenderer_ptr(_renderer);
    if (_window) SDL_DestroyWindow_ptr(_window);
    if (_michaelModeTexture) SDL_DestroyTexture_ptr(_michaelModeTexture);
    SDL_Quit_ptr();
}

void SDL3Game::display_good_part(SDL_FRect rect, int currentDirection, const std::vector<std::pair<int, int>>& snakeBody, bool modeMichael) {
    const float michaelOffsetX = modeMichael ? 640.0f : 0.0f;
    const float michaelOffsetY = modeMichael ? 350.0f : 0.0f;

    auto renderRotated = [&](SDL_Texture* texture, double angle) {
        SDL_FPoint center = { rect.w / 2.0f, rect.h / 2.0f };
        SDL_RenderTextureRotated_ptr(_renderer, texture, nullptr, &rect, angle, &center, 0);
    };

    for (size_t i = 0; i < snakeBody.size(); ++i) {
        rect.x = static_cast<float>(snakeBody[i].first * 32) + michaelOffsetX;
        rect.y = static_cast<float>(snakeBody[i].second * 32) + michaelOffsetY;

        auto segmentAngleFromDelta = [](int dx, int dy) -> double {
            if (dx == 1) return 0.0;
            if (dx == -1) return 180.0;
            if (dy == 1) return 90.0;
            if (dy == -1) return 270.0;
            return 0.0;
        };

        // HEAD
        if (i == 0) {
            double angle = 0.0;
            if (currentDirection == LEFT) {
                angle = 180.0;
            } else if (currentDirection == UP) {
                angle = 270.0;
            } else if (currentDirection == DOWN) {
                angle = 90.0;
            }
            renderRotated(_snakeLeftRightTexture, angle);
        }

        // TAIL (optionnel à compléter)
        else if (i == snakeBody.size() - 1) {
            auto prev = snakeBody[i - 1];
            auto tail = snakeBody[i];
            int dx = tail.first - prev.first;
            int dy = tail.second - prev.second;
            renderRotated(_snakeLeftRightTexture, segmentAngleFromDelta(dx, dy));
            std::cout << "Tail segment at (" << tail.first << ", " << tail.second << ") with delta (" << dx << ", " << dy << ")" << std::endl;
            std::cout << "Tail segment angle: " << segmentAngleFromDelta(dx, dy) << " degrees" << std::endl;
            std::cout << "Tail segment texture: " << (_snakeLeftRightTexture ? "Loaded" : "Not Loaded") << std::endl;
            std::cerr << "previous Snake size: " << snakeBody.size() << " i " << i - 1 << " current position: (" << snakeBody[i - 1].first << ", " << snakeBody[i - 1].second << ")" << std::endl;
            std::cerr << "prvious previous Snake size: " << snakeBody.size() << " i " << i - 2 << " current position: (" << snakeBody[i - 2].first << ", " << snakeBody[i - 2].second << ")" << std::endl;
            std::cerr << "Snake size: " << snakeBody.size() << " i " << i << "current position: (" << snakeBody[i].first << ", " << snakeBody[i].second << ")" << std::endl;
        }

        // BODY
        else {
            auto prev = snakeBody[i - 1];
            auto curr = snakeBody[i];
            auto next = snakeBody[i + 1];

            int dx1 = prev.first - curr.first;
            int dy1 = prev.second - curr.second;

            int dx2 = next.first - curr.first;
            int dy2 = next.second - curr.second;

            // STRAIGHT
            if ((dx1 == dx2) || (dy1 == dy2)) {
                if (dx1 != 0)
                    renderRotated(_snakeLeftRightTexture, 0);
                else
                    renderRotated(_snakeUpDownTexture, 0);
            }

            // TURN
            else {
                double angle = 0;

                // Haut + droite
                if ((dy1 == -1 && dx2 == 1) || (dx1 == 1 && dy2 == -1))
                    angle = 90;

                // Droite + bas
                else if ((dx1 == 1 && dy2 == 1) || (dy1 == 1 && dx2 == 1))
                    angle = 180;

                // Bas + gauche
                else if ((dy1 == 1 && dx2 == -1) || (dx1 == -1 && dy2 == 1))
                    angle = 270;

                // Gauche + haut
                else if ((dx1 == -1 && dy2 == -1) || (dy1 == -1 && dx2 == -1))
                    angle = 0;

                renderRotated(_snakeTurnLeftTexture, angle);
            }
        }
    }
}

void SDL3Game::display(const Game& game) {
    const float michaelOffsetX = game.getMichaelMode() ? 640.0f : 0.0f;
    const float michaelOffsetY = game.getMichaelMode() ? 350.0f : 0.0f;

    SDL_SetRenderDrawColor_ptr(_renderer, 0, 0, 0, 255);
    SDL_RenderClear_ptr(_renderer);

    // if (game.getMichaelMode() && _michaelModeTexture) {
    //     SDL_FRect rect = { 0.0f, 0.0f, 1400.0f, 1800.0f };
    //     SDL_RenderTexture_ptr(_renderer, _michaelModeTexture, nullptr, &rect);
    // }

    for (int y = 0; y < _height; ++y) {
        for (int x = 0; x < _width; ++x) {
            SDL_FRect rect = {
                static_cast<float>(x * 32) + michaelOffsetX,
                static_cast<float>(y * 32) + michaelOffsetY,
                32.0f,
                32.0f
            };

            if (_backgroundTexture) {
                SDL_RenderTexture_ptr(_renderer, _backgroundTexture, nullptr, &rect);
            } else {
                SDL_SetRenderDrawColor_ptr(_renderer, 50, 50, 50, 255);
                SDL_RenderFillRect_ptr(_renderer, &rect);
            }
            
            int cell = game.getCell(x, y);
            if (cell == FOOD) {
                if (_foodTexture) {
                    SDL_RenderTexture_ptr(_renderer, _foodTexture, nullptr, &rect);
                } else {
                    SDL_SetRenderDrawColor_ptr(_renderer, 255, 0, 0, 255);
                    SDL_RenderFillRect_ptr(_renderer, &rect);
                }
            } else if (cell == WALL) {
                if (_wallTexture) {
                    SDL_RenderTexture_ptr(_renderer, _wallTexture, nullptr, &rect);
                } else {
                    SDL_SetRenderDrawColor_ptr(_renderer, 128, 128, 128, 255);
                    SDL_RenderFillRect_ptr(_renderer, &rect);
                }
            }
        }
    }

    if (_snakeUpDownTexture && _snakeLeftRightTexture && _snakeTurnRightTexture && _snakeTurnLeftTexture) {
        SDL_FRect snakeRect = { 0.0f, 0.0f, 32.0f, 32.0f };
        display_good_part(snakeRect, game.getCurrentDirection(), game.getSnakeBody(), game.getMichaelMode());
    }

    if (game.getMichaelMode() && _michaelModeTexture) {
        SDL_FRect rect = { 0.0f, 0.0f, 1400.0f, 1800.0f };
        SDL_RenderTexture_ptr(_renderer, _michaelModeTexture, nullptr, &rect);
    }

    SDL_RenderPresent_ptr(_renderer);
}

int SDL3Game::handleInput() {
    SDL_Event event;
    
    while (SDL_PollEvent_ptr(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            std::cerr << "[SDL3] SDL_EVENT_QUIT received" << std::endl;
            return -1;
        }
        
        if (event.type == SDL_EVENT_KEY_DOWN) {
            // Direct access to scancode at offset 24 in the structure
            unsigned char* bytes = (unsigned char*)&event;
            int scancode = *(int*)(bytes + 24);  // offset 24 = scancode (little-endian int)
            
            if (scancode == 19) {
                
                return 1000;                
            }
            std::cout << "[SDL3] Key down event: scancode=" << scancode << std::endl;
            if (scancode == SDL_SCANCODE_ESCAPE) {
                return -1;
            }
            // Arrow keys
            if (scancode == SDL_SCANCODE_LEFT) {
                return LEFT;
            }
            if (scancode == SDL_SCANCODE_RIGHT) {
                return RIGHT;
            }
            if (scancode == SDL_SCANCODE_UP) {
                return UP;
            }
            if (scancode == SDL_SCANCODE_DOWN) {
                return DOWN;
            }
            // Mode controls (use values > 4 to avoid conflict with Direction enum)
            if (scancode == SDL_SCANCODE_1) {
                currentLibrary = SDL3;
                return 10;  // Increased to avoid UP (1), DOWN (2), etc.
            }
            if (scancode == SDL_SCANCODE_2) {
                currentLibrary = SFML;
                return 20;
            }
            if (scancode == SDL_SCANCODE_3) {
                currentLibrary = GL;
                return 30;
            }
        }
    }
    
    return 0;
}

extern "C" {
    void* create_gui_sdl3(int width, int height, bool michaelMode) {
        return new SDL3Game(width, height, michaelMode);
    }
    
    void destroy_gui_sdl3(void* gui) {
        delete (SDL3Game*)gui;
    }
    
    void display_gui_sdl3(void* gui, const Game& game) {
        ((SDL3Game*)gui)->display(game);
    }
    
    int input_gui_sdl3(void* gui) {
        return ((SDL3Game*)gui)->handleInput();
    }
}