#include "../includes/GlGame.hpp"

#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_PROJECTION 0x1701
#define GL_MODELVIEW 0x1700
#define GL_QUADS 0x0007
#define GL_LINES 0x0001

typedef void (*PFNGLCLEARCOLORPROC)(float, float, float, float);
typedef void (*PFNGLCLEARPROC)(unsigned int);
typedef void (*PFNGLVIEWPORTPROC)(int, int, int, int);
typedef void (*PFNGLMATRIXMODEPROC)(unsigned int);
typedef void (*PFNGLLOADIDENTITYPROC)(void);
typedef void (*PFNGLORTHOPROC)(double, double, double, double, double, double);
typedef void (*PFNGLBEGINPROC)(unsigned int);
typedef void (*PFNGLENDPROC)(void);
typedef void (*PFNGLCOLOR3FPROC)(float, float, float);
typedef void (*PFNGLVERTEX2FPROC)(float, float);

typedef int (*PFNGLFWINITPROC)(void);
typedef void (*PFNGLFWTERMINATEPROC)(void);
typedef GLFWwindow* (*PFNGLFWCREATEWINDOWPROC)(int, int, const char*, GLFWmonitor*, GLFWwindow*);
typedef void (*PFNGLFWDESTROYWINDOWPROC)(GLFWwindow*);
typedef void (*PFNGLFWMAKECONTEXTCURRENTPROC)(GLFWwindow*);
typedef void (*PFNGLFWSWAPINTERVALPROC)(int);
typedef void (*PFNGLFWSWAPBUFFERSPROC)(GLFWwindow*);
typedef void (*PFNGLFWPOLLEVENTSPROC)(void);
typedef int (*PFNGLFWWINDOWSHOULDCLOSEPROC)(GLFWwindow*);
typedef void (*PFNGLFWSETWINDOWSHOULDCLOSEPROC)(GLFWwindow*, int);
typedef GLFWkeyfun (*PFNGLFWSETKEYCALLBACKPROC)(GLFWwindow*, GLFWkeyfun);
typedef void (*PFNGLFWGETFRAMEBUFFERSIZEPROC)(GLFWwindow*, int*, int*);
typedef void (*PFNGLFWSETWINDOWSIZELIMITSPROC)(GLFWwindow*, int, int, int, int);

static void* gl_handle = nullptr;
static void* glfw_handle = nullptr;
static bool symbols_loaded = false;
static bool glfw_initialized = false;

static PFNGLCLEARCOLORPROC glClearColor_ptr = nullptr;
static PFNGLCLEARPROC glClear_ptr = nullptr;
static PFNGLVIEWPORTPROC glViewport_ptr = nullptr;
static PFNGLMATRIXMODEPROC glMatrixMode_ptr = nullptr;
static PFNGLLOADIDENTITYPROC glLoadIdentity_ptr = nullptr;
static PFNGLORTHOPROC glOrtho_ptr = nullptr;
static PFNGLBEGINPROC glBegin_ptr = nullptr;
static PFNGLENDPROC glEnd_ptr = nullptr;
static PFNGLCOLOR3FPROC glColor3f_ptr = nullptr;
static PFNGLVERTEX2FPROC glVertex2f_ptr = nullptr;

static PFNGLFWINITPROC glfwInit_ptr = nullptr;
static PFNGLFWTERMINATEPROC glfwTerminate_ptr = nullptr;
static PFNGLFWCREATEWINDOWPROC glfwCreateWindow_ptr = nullptr;
static PFNGLFWDESTROYWINDOWPROC glfwDestroyWindow_ptr = nullptr;
static PFNGLFWMAKECONTEXTCURRENTPROC glfwMakeContextCurrent_ptr = nullptr;
static PFNGLFWSWAPINTERVALPROC glfwSwapInterval_ptr = nullptr;
static PFNGLFWSWAPBUFFERSPROC glfwSwapBuffers_ptr = nullptr;
static PFNGLFWPOLLEVENTSPROC glfwPollEvents_ptr = nullptr;
static PFNGLFWWINDOWSHOULDCLOSEPROC glfwWindowShouldClose_ptr = nullptr;
static PFNGLFWSETWINDOWSHOULDCLOSEPROC glfwSetWindowShouldClose_ptr = nullptr;
static PFNGLFWSETKEYCALLBACKPROC glfwSetKeyCallback_ptr = nullptr;
static PFNGLFWGETFRAMEBUFFERSIZEPROC glfwGetFramebufferSize_ptr = nullptr;
static PFNGLFWSETWINDOWSIZELIMITSPROC glfwSetWindowSizeLimits_ptr = nullptr;

static std::map<int, bool> edge_pressed;
static bool close_requested = false;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)window;
    (void)scancode;
    (void)mods;

    if (action == GLFW_PRESS) {
        edge_pressed[key] = true;
        if (key == GLFW_KEY_ESCAPE) {
            close_requested = true;
        }
    }
}

static int directionToP2Action(Direction direction) {
    switch (direction) {
        case UP:
            return P2_UP;
        case DOWN:
            return P2_DOWN;
        case LEFT:
            return P2_LEFT;
        case RIGHT:
        default:
            return P2_RIGHT;
    }
}

static void unload_symbols() {
    if (glfw_handle) {
        dlclose(glfw_handle);
        glfw_handle = nullptr;
    }
    if (gl_handle) {
        dlclose(gl_handle);
        gl_handle = nullptr;
    }
    symbols_loaded = false;
}

static void* open_lib(const char* const* names, std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) {
        void* handle = dlopen(names[i], RTLD_LAZY | RTLD_GLOBAL);
        if (handle) {
            return handle;
        }
    }
    return nullptr;
}

static bool load_glfw_gl_symbols() {
    if (symbols_loaded) {
        return true;
    }

    const char* gl_names[] = {"libGL.so.1", "libGL.so", "/usr/lib/x86_64-linux-gnu/libGL.so.1"};
    gl_handle = open_lib(gl_names, sizeof(gl_names) / sizeof(gl_names[0]));
    if (!gl_handle) {
        std::cerr << "[GL] Could not load OpenGL library" << std::endl;
        return false;
    }

    const char* glfw_names[] = {"./glfw-3.4/build/src/libglfw.so", "./glfw-3.4/build/src/libglfw.so.3", "libglfw.so.3", "libglfw.so"};
    glfw_handle = open_lib(glfw_names, sizeof(glfw_names) / sizeof(glfw_names[0]));
    if (!glfw_handle) {
        std::cerr << "[GL] Could not load GLFW library: " << dlerror() << std::endl;
        unload_symbols();
        return false;
    }

    glfwInit_ptr = reinterpret_cast<PFNGLFWINITPROC>(dlsym(glfw_handle, "glfwInit"));
    glfwTerminate_ptr = reinterpret_cast<PFNGLFWTERMINATEPROC>(dlsym(glfw_handle, "glfwTerminate"));
    glfwCreateWindow_ptr = reinterpret_cast<PFNGLFWCREATEWINDOWPROC>(dlsym(glfw_handle, "glfwCreateWindow"));
    glfwDestroyWindow_ptr = reinterpret_cast<PFNGLFWDESTROYWINDOWPROC>(dlsym(glfw_handle, "glfwDestroyWindow"));
    glfwMakeContextCurrent_ptr = reinterpret_cast<PFNGLFWMAKECONTEXTCURRENTPROC>(dlsym(glfw_handle, "glfwMakeContextCurrent"));
    glfwSwapInterval_ptr = reinterpret_cast<PFNGLFWSWAPINTERVALPROC>(dlsym(glfw_handle, "glfwSwapInterval"));
    glfwSwapBuffers_ptr = reinterpret_cast<PFNGLFWSWAPBUFFERSPROC>(dlsym(glfw_handle, "glfwSwapBuffers"));
    glfwPollEvents_ptr = reinterpret_cast<PFNGLFWPOLLEVENTSPROC>(dlsym(glfw_handle, "glfwPollEvents"));
    glfwWindowShouldClose_ptr = reinterpret_cast<PFNGLFWWINDOWSHOULDCLOSEPROC>(dlsym(glfw_handle, "glfwWindowShouldClose"));
    glfwSetWindowShouldClose_ptr = reinterpret_cast<PFNGLFWSETWINDOWSHOULDCLOSEPROC>(dlsym(glfw_handle, "glfwSetWindowShouldClose"));
    glfwSetKeyCallback_ptr = reinterpret_cast<PFNGLFWSETKEYCALLBACKPROC>(dlsym(glfw_handle, "glfwSetKeyCallback"));
    glfwGetFramebufferSize_ptr = reinterpret_cast<PFNGLFWGETFRAMEBUFFERSIZEPROC>(dlsym(glfw_handle, "glfwGetFramebufferSize"));
    glfwSetWindowSizeLimits_ptr = reinterpret_cast<PFNGLFWSETWINDOWSIZELIMITSPROC>(dlsym(glfw_handle, "glfwSetWindowSizeLimits"));

    glClearColor_ptr = reinterpret_cast<PFNGLCLEARCOLORPROC>(dlsym(gl_handle, "glClearColor"));
    glClear_ptr = reinterpret_cast<PFNGLCLEARPROC>(dlsym(gl_handle, "glClear"));
    glViewport_ptr = reinterpret_cast<PFNGLVIEWPORTPROC>(dlsym(gl_handle, "glViewport"));
    glMatrixMode_ptr = reinterpret_cast<PFNGLMATRIXMODEPROC>(dlsym(gl_handle, "glMatrixMode"));
    glLoadIdentity_ptr = reinterpret_cast<PFNGLLOADIDENTITYPROC>(dlsym(gl_handle, "glLoadIdentity"));
    glOrtho_ptr = reinterpret_cast<PFNGLORTHOPROC>(dlsym(gl_handle, "glOrtho"));
    glBegin_ptr = reinterpret_cast<PFNGLBEGINPROC>(dlsym(gl_handle, "glBegin"));
    glEnd_ptr = reinterpret_cast<PFNGLENDPROC>(dlsym(gl_handle, "glEnd"));
    glColor3f_ptr = reinterpret_cast<PFNGLCOLOR3FPROC>(dlsym(gl_handle, "glColor3f"));
    glVertex2f_ptr = reinterpret_cast<PFNGLVERTEX2FPROC>(dlsym(gl_handle, "glVertex2f"));

    if (!glfwInit_ptr || !glfwTerminate_ptr || !glfwCreateWindow_ptr || !glfwDestroyWindow_ptr ||
        !glfwMakeContextCurrent_ptr || !glfwSwapInterval_ptr || !glfwSwapBuffers_ptr ||
        !glfwPollEvents_ptr || !glfwWindowShouldClose_ptr || !glfwSetWindowShouldClose_ptr ||
        !glfwSetKeyCallback_ptr || !glfwGetFramebufferSize_ptr || !glfwSetWindowSizeLimits_ptr ||
        !glClearColor_ptr || !glClear_ptr ||
        !glViewport_ptr || !glMatrixMode_ptr || !glLoadIdentity_ptr || !glOrtho_ptr ||
        !glBegin_ptr || !glEnd_ptr || !glColor3f_ptr || !glVertex2f_ptr) {
        std::cerr << "[GL] Missing required OpenGL/GLFW symbols" << std::endl;
        unload_symbols();
        return false;
    }

    symbols_loaded = true;
    return true;
}

void GLGame::drawRect(float x, float y, float w, float h, float r, float g, float b) const {
    glColor3f_ptr(r, g, b);
    glBegin_ptr(GL_QUADS);
    glVertex2f_ptr(x, y);
    glVertex2f_ptr(x + w, y);
    glVertex2f_ptr(x + w, y + h);
    glVertex2f_ptr(x, y + h);
    glEnd_ptr();
}

void GLGame::drawQuad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
    float r, float g, float b) const {
    glColor3f_ptr(r, g, b);
    glBegin_ptr(GL_QUADS);
    glVertex2f_ptr(x0, y0);
    glVertex2f_ptr(x1, y1);
    glVertex2f_ptr(x2, y2);
    glVertex2f_ptr(x3, y3);
    glEnd_ptr();
}

void GLGame::drawLine(float x0, float y0, float x1, float y1, float r, float g, float b) const {
    glColor3f_ptr(r, g, b);
    glBegin_ptr(GL_LINES);
    glVertex2f_ptr(x0, y0);
    glVertex2f_ptr(x1, y1);
    glEnd_ptr();
}

bool GLGame::isSolidCell(const Game& game, int x, int y) {
    const int cell = game.getCell(x, y);
    return cell == SNAKE || cell == SNAKE_2 || cell == WALL || cell == FOOD;
}

static bool shouldRenderApple(const Game& game) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    long elapsed = (now.tv_sec - game.getSpawnApple().tv_sec) * 1000 +
        (now.tv_usec - game.getSpawnApple().tv_usec) / 1000;

    if (game.getAppelDespawned() && elapsed > (game.getWidth() + game.getHeight()) * 16) {
        return false;
    }
    return true;
}

void GLGame::renderProjectedFloorGrid(double posX, double posY, double dirX, double dirY, int fbw, int fbh) const {
    struct ProjectedTile {
        float x0;
        float y0;
        float x1;
        float y1;
        float x2;
        float y2;
        float x3;
        float y3;
        float depth;
        bool major;
        bool checker;
    };

    const int majorStep = 5;
    const float horizon = static_cast<float>(fbh) * 0.5f;
    const float halfW = static_cast<float>(fbw) * 0.5f;
    const float projX = static_cast<float>(fbw) / (2.0f * 0.66f);
    const float projY = static_cast<float>(fbh) / (2.0f * 0.66f);
    const double camHeight = 0.5;

    const double rightX = -dirY;
    const double rightY = dirX;

    std::vector<ProjectedTile> tiles;
    tiles.reserve(static_cast<size_t>(_width) * static_cast<size_t>(_height));

    for (int y = 0; y < _height; ++y) {
        for (int x = 0; x < _width; ++x) {
            const double wx[4] = {
                static_cast<double>(x),
                static_cast<double>(x + 1),
                static_cast<double>(x + 1),
                static_cast<double>(x)
            };
            const double wy[4] = {
                static_cast<double>(y),
                static_cast<double>(y),
                static_cast<double>(y + 1),
                static_cast<double>(y + 1)
            };

            double depth[4];
            float sx[4];
            float sy[4];
            bool visible = true;

            for (int i = 0; i < 4; ++i) {
                const double relX = wx[i] - posX;
                const double relY = wy[i] - posY;
                depth[i] = relX * dirX + relY * dirY;
                if (depth[i] <= 0.08) {
                    visible = false;
                    break;
                }

                const double side = relX * rightX + relY * rightY;
                sx[i] = halfW + static_cast<float>((side / depth[i]) * static_cast<double>(projX));
                sy[i] = horizon + static_cast<float>((camHeight / depth[i]) * static_cast<double>(projY));
            }

            if (!visible) {
                continue;
            }

            const float maxY = std::max(std::max(sy[0], sy[1]), std::max(sy[2], sy[3]));
            const float minY = std::min(std::min(sy[0], sy[1]), std::min(sy[2], sy[3]));
            const float maxX = std::max(std::max(sx[0], sx[1]), std::max(sx[2], sx[3]));
            const float minX = std::min(std::min(sx[0], sx[1]), std::min(sx[2], sx[3]));

            if (maxY < horizon + 1.0f || minY > static_cast<float>(fbh) ||
                maxX < 0.0f || minX > static_cast<float>(fbw)) {
                continue;
            }

            ProjectedTile tile;
            tile.x0 = sx[0];
            tile.y0 = sy[0];
            tile.x1 = sx[1];
            tile.y1 = sy[1];
            tile.x2 = sx[2];
            tile.y2 = sy[2];
            tile.x3 = sx[3];
            tile.y3 = sy[3];
            tile.depth = static_cast<float>((depth[0] + depth[1] + depth[2] + depth[3]) * 0.25);
            tile.major = (x % majorStep == 0) || (y % majorStep == 0);
            tile.checker = ((x + y) % 2 == 0);
            tiles.push_back(tile);
        }
    }

    std::sort(tiles.begin(), tiles.end(), [](const ProjectedTile& a, const ProjectedTile& b) {
        return a.depth > b.depth;
    });

    for (size_t i = 0; i < tiles.size(); ++i) {
        const ProjectedTile& tile = tiles[i];
        const float shade = std::max(0.18f, 1.0f / (1.0f + tile.depth * 0.07f));
        const float checker = tile.checker ? 1.00f : 0.94f;
        const float floorTone = 0.11f * shade * checker;
        drawQuad(tile.x0, tile.y0, tile.x1, tile.y1, tile.x2, tile.y2, tile.x3, tile.y3,
            floorTone, floorTone * 0.92f, floorTone * 0.82f);

        const float edgeColor = tile.major ? 0.0f : 0.03f;
        drawLine(tile.x0, tile.y0, tile.x1, tile.y1, edgeColor, edgeColor, edgeColor);
        drawLine(tile.x1, tile.y1, tile.x2, tile.y2, edgeColor, edgeColor, edgeColor);
        drawLine(tile.x2, tile.y2, tile.x3, tile.y3, edgeColor, edgeColor, edgeColor);
        drawLine(tile.x3, tile.y3, tile.x0, tile.y0, edgeColor, edgeColor, edgeColor);
    }
}

Direction GLGame::turnLeft(Direction dir) {
    switch (dir) {
        case UP:
            return LEFT;
        case DOWN:
            return RIGHT;
        case LEFT:
            return DOWN;
        case RIGHT:
        default:
            return UP;
    }
}

Direction GLGame::turnRight(Direction dir) {
    switch (dir) {
        case UP:
            return RIGHT;
        case DOWN:
            return LEFT;
        case LEFT:
            return UP;
        case RIGHT:
        default:
            return DOWN;
    }
}

void GLGame::renderRaycast(const Game& game, int fbw, int fbh) const {
    const std::vector<std::pair<int, int>>& snake = game.getSnakeBody();
    if (snake.empty()) {
        return;
    }

    const std::pair<int, int>& head = snake.front();
    const double posX = static_cast<double>(head.first) + 0.5;
    const double posY = static_cast<double>(head.second) + 0.5;

    double dirX = 1.0;
    double dirY = 0.0;
    switch (game.getCurrentDirection()) {
        case UP:
            dirX = 0.0;
            dirY = -1.0;
            break;
        case DOWN:
            dirX = 0.0;
            dirY = 1.0;
            break;
        case LEFT:
            dirX = -1.0;
            dirY = 0.0;
            break;
        case RIGHT:
        default:
            dirX = 1.0;
            dirY = 0.0;
            break;
    }

    const double fovScale = 0.66;
    const double planeX = -dirY * fovScale;
    const double planeY = dirX * fovScale;

    const float horizon = static_cast<float>(fbh) * 0.5f;
    drawRect(0.0f, 0.0f, static_cast<float>(fbw), horizon, 0.10f, 0.12f, 0.16f);
    drawRect(0.0f, horizon, static_cast<float>(fbw), static_cast<float>(fbh) - horizon, 0.10f, 0.08f, 0.06f);
    renderProjectedFloorGrid(posX, posY, dirX, dirY, fbw, fbh);

    for (int x = 0; x < fbw; ++x) {
        const double cameraX = 2.0 * static_cast<double>(x) / static_cast<double>(fbw) - 1.0;
        const double rayDirX = dirX + planeX * cameraX;
        const double rayDirY = dirY + planeY * cameraX;

        int mapX = static_cast<int>(std::floor(posX));
        int mapY = static_cast<int>(std::floor(posY));

        const double deltaDistX = (rayDirX == 0.0) ? 1e30 : std::fabs(1.0 / rayDirX);
        const double deltaDistY = (rayDirY == 0.0) ? 1e30 : std::fabs(1.0 / rayDirY);

        int stepX = 0;
        int stepY = 0;
        double sideDistX = 0.0;
        double sideDistY = 0.0;

        if (rayDirX < 0.0) {
            stepX = -1;
            sideDistX = (posX - static_cast<double>(mapX)) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (static_cast<double>(mapX) + 1.0 - posX) * deltaDistX;
        }
        if (rayDirY < 0.0) {
            stepY = -1;
            sideDistY = (posY - static_cast<double>(mapY)) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (static_cast<double>(mapY) + 1.0 - posY) * deltaDistY;
        }

        bool hit = false;
        int side = 0;
        int hitCell = WALL;
        int maxSteps = (_width + _height) * 4;
        while (!hit && maxSteps-- > 0) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            hitCell = game.getCell(mapX, mapY);
            if (hitCell == FOOD && !shouldRenderApple(game)) {
                continue;
            }
            if (hitCell == SNAKE || hitCell == SNAKE_2 || hitCell == WALL || hitCell == FOOD) {
                hit = true;
            }
        }

        double perpWallDist = 1e30;
        if (hit) {
            if (side == 0) {
                perpWallDist = (static_cast<double>(mapX) - posX + (1.0 - static_cast<double>(stepX)) * 0.5) /
                    ((rayDirX == 0.0) ? 1.0 : rayDirX);
            } else {
                perpWallDist = (static_cast<double>(mapY) - posY + (1.0 - static_cast<double>(stepY)) * 0.5) /
                    ((rayDirY == 0.0) ? 1.0 : rayDirY);
            }
        }
        perpWallDist = std::fabs(perpWallDist);
        if (perpWallDist < 0.0001) {
            perpWallDist = 0.0001;
        }

        int lineHeight = static_cast<int>(static_cast<double>(fbh) / perpWallDist);
        int drawStart = -lineHeight / 2 + fbh / 2;
        int drawEnd = lineHeight / 2 + fbh / 2;
        if (drawStart < 0) {
            drawStart = 0;
        }
        if (drawEnd >= fbh) {
            drawEnd = fbh - 1;
        }

        float shade = static_cast<float>(1.0 / (1.0 + perpWallDist * 0.10));
        if (shade < 0.15f) {
            shade = 0.15f;
        }
        const float sideShade = (side == 1) ? 0.75f : 1.0f;
        float wallR = 0.20f * shade * sideShade;
        float wallG = 0.72f * shade * sideShade;
        float wallB = 0.30f * shade * sideShade;
        if (hitCell == FOOD) {
            wallR = 0.90f * shade * sideShade;
            wallG = 0.18f * shade * sideShade;
            wallB = 0.14f * shade * sideShade;
        } else if (hitCell == WALL) {
            wallR = 0.64f * shade * sideShade;
            wallG = 0.64f * shade * sideShade;
            wallB = 0.64f * shade * sideShade;
        }

        drawRect(static_cast<float>(x), static_cast<float>(drawStart), 1.0f,
            static_cast<float>(drawEnd - drawStart + 1), wallR, wallG, wallB);
    }
}

void GLGame::renderMiniMap(const Game& game, int fbw, int fbh) const {
    const int margin = 16;
    const int minimapMaxW = fbw / 3;
    const int minimapMaxH = fbh / 3;
    const float cellSize = std::max(4.0f,
        std::min(static_cast<float>(minimapMaxW) / static_cast<float>(_width),
            static_cast<float>(minimapMaxH) / static_cast<float>(_height)));
    const float mapW = cellSize * static_cast<float>(_width);
    const float mapH = cellSize * static_cast<float>(_height);
    const float mapX = static_cast<float>(fbw) - mapW - static_cast<float>(margin);
    const float mapY = static_cast<float>(margin);

    drawRect(mapX - 8.0f, mapY - 8.0f, mapW + 16.0f, mapH + 16.0f, 0.03f, 0.03f, 0.04f);

    for (int y = 0; y < _height; ++y) {
        for (int x = 0; x < _width; ++x) {
            const float px = mapX + static_cast<float>(x) * cellSize;
            const float py = mapY + static_cast<float>(y) * cellSize;

            drawRect(px, py, cellSize - 1.0f, cellSize - 1.0f, 0.15f, 0.16f, 0.18f);

            const int cell = game.getCell(x, y);
            if (cell == FOOD) {
                if (shouldRenderApple(game)) {
                    drawRect(px + 2.0f, py + 2.0f, cellSize - 4.0f, cellSize - 4.0f, 0.90f, 0.20f, 0.16f);
                }
            } else if (cell == SNAKE || cell == SNAKE_2) {
                drawRect(px + 1.0f, py + 1.0f, cellSize - 2.0f, cellSize - 2.0f, 0.16f, 0.78f, 0.25f);
            } else if (cell == WALL) {
                drawRect(px + 1.0f, py + 1.0f, cellSize - 2.0f, cellSize - 2.0f, 0.60f, 0.60f, 0.60f);
            }
        }
    }

    const std::vector<std::pair<int, int>>& snake = game.getSnakeBody();
    if (!snake.empty()) {
        const std::pair<int, int>& head = snake.front();
        const float centerX = mapX + (static_cast<float>(head.first) + 0.5f) * cellSize;
        const float centerY = mapY + (static_cast<float>(head.second) + 0.5f) * cellSize;
        float dirX = 1.0f;
        float dirY = 0.0f;
        switch (game.getCurrentDirection()) {
            case UP:
                dirX = 0.0f;
                dirY = -1.0f;
                break;
            case DOWN:
                dirX = 0.0f;
                dirY = 1.0f;
                break;
            case LEFT:
                dirX = -1.0f;
                dirY = 0.0f;
                break;
            case RIGHT:
            default:
                dirX = 1.0f;
                dirY = 0.0f;
                break;
        }

        drawRect(centerX - 1.5f, centerY - 1.5f, 3.0f, 3.0f, 1.0f, 1.0f, 0.85f);
        drawRect(centerX, centerY, dirX * cellSize * 0.9f + 1.0f, dirY * cellSize * 0.9f + 1.0f,
            1.0f, 1.0f, 0.2f);
    }
}

GLGame::GLGame()
    : GLGame(20, 15, false) {
}

GLGame::GLGame(int w, int h, bool michaelMode)
    : _window(nullptr), _width(std::max(10, w)), _height(std::max(10, h)), _cell_size(32), _lastDirection(RIGHT), _lastDirection2(LEFT) {
    if (!load_glfw_gl_symbols()) {
        return;
    }

    _michaelMode = michaelMode;
    (void)_cell_size;

    if (!glfw_initialized) {
        if (!glfwInit_ptr()) {
            std::cerr << "[GL] glfwInit failed" << std::endl;
            return;
        }
        glfw_initialized = true;
    }

    const int pixel_width = WINDOW_WIDTH;
    const int pixel_height = WINDOW_HEIGHT;
    _window = glfwCreateWindow_ptr(pixel_width, pixel_height, "Nibbler - OpenGL", nullptr, nullptr);
    if (!_window) {
        std::cerr << "[GL] glfwCreateWindow failed" << std::endl;
        return;
    }

    glfwSetWindowSizeLimits_ptr(_window, pixel_width, pixel_height, pixel_width, pixel_height);

    glfwMakeContextCurrent_ptr(_window);
    glfwSwapInterval_ptr(1);
    glfwSetKeyCallback_ptr(_window, key_callback);

    glClearColor_ptr(0.09f, 0.10f, 0.12f, 1.0f);
}

GLGame::~GLGame() {
    if (_window && glfwDestroyWindow_ptr) {
        glfwDestroyWindow_ptr(_window);
        _window = nullptr;
    }
    if (glfw_initialized && glfwTerminate_ptr) {
        glfwTerminate_ptr();
        glfw_initialized = false;
    }
    unload_symbols();
}

GLGame::GLGame(const GLGame& other) {
    if (this != &other) {
        _window = nullptr;
        _width = other._width;
        _height = other._height;
        _cell_size = other._cell_size;
        _lastDirection = other._lastDirection;
        _lastDirection2 = other._lastDirection2;

        if (other.isReady()) {
            *this = GLGame(_width, _height, other._michaelMode);
        }
    }
}

GLGame& GLGame::operator=(const GLGame& other) {
    if (this != &other) {
        GLGame temp(other);
        std::swap(_window, temp._window);
        std::swap(_width, temp._width);
        std::swap(_height, temp._height);
        std::swap(_cell_size, temp._cell_size);
        std::swap(_lastDirection, temp._lastDirection);
        std::swap(_lastDirection2, temp._lastDirection2);
        std::swap(_michaelMode, temp._michaelMode);
    }
    return *this;
}

bool GLGame::isReady() const {
    return _window != nullptr;
}

void GLGame::display(const Game& game) {
    if (!isReady()) {
        return;
    }

    glfwMakeContextCurrent_ptr(_window);

    int fbw = 0;
    int fbh = 0;
    glfwGetFramebufferSize_ptr(_window, &fbw, &fbh);
    glViewport_ptr(0, 0, fbw, fbh);

    glMatrixMode_ptr(GL_PROJECTION);
    glLoadIdentity_ptr();
    glOrtho_ptr(0.0, static_cast<double>(fbw), static_cast<double>(fbh), 0.0, -1.0, 1.0);
    glMatrixMode_ptr(GL_MODELVIEW);
    glLoadIdentity_ptr();

    glClear_ptr(GL_COLOR_BUFFER_BIT);

    _lastDirection = static_cast<Direction>(game.getCurrentDirection());
    if (game.getNbPlayer() >= 2) {
        _lastDirection2 = static_cast<Direction>(game.getCurrentDirection2());
    }

    renderRaycast(game, fbw, fbh);
    renderMiniMap(game, fbw, fbh);

    glfwSwapBuffers_ptr(_window);
}

int GLGame::handleInput() {
    if (!isReady()) {
        return -1;
    }

    glfwPollEvents_ptr();

    if (close_requested || glfwWindowShouldClose_ptr(_window)) {
        close_requested = false;
        return -1;
    }

    if (edge_pressed[GLFW_KEY_ESCAPE]) {
        edge_pressed.clear();
        close_requested = false;
        glfwSetWindowShouldClose_ptr(_window, 1);
        return -1;
    }
    if (edge_pressed[GLFW_KEY_1]) {
        edge_pressed.clear();
        currentLibrary = SDL3;
        return 10;
    }
    if (edge_pressed[GLFW_KEY_2]) {
        edge_pressed.clear();
        currentLibrary = SFML;
        return 20;
    }
    if (edge_pressed[GLFW_KEY_3]) {
        edge_pressed.clear();
        currentLibrary = GL;
        return 30;
    }
    if (edge_pressed[GLFW_KEY_LEFT]) {
        edge_pressed.clear();
        return turnLeft(_lastDirection);
    }
    if (edge_pressed[GLFW_KEY_RIGHT]) {
        edge_pressed.clear();
        return turnRight(_lastDirection);
    }
    if (edge_pressed[GLFW_KEY_A]) {
        const Direction nextDirection = turnLeft(_lastDirection2);
        edge_pressed.clear();
        return directionToP2Action(nextDirection);
    }
    if (edge_pressed[GLFW_KEY_D]) {
        const Direction nextDirection = turnRight(_lastDirection2);
        edge_pressed.clear();
        return directionToP2Action(nextDirection);
    }

    edge_pressed.clear();
    return 0;
}

extern "C" {
    void* create_gui_gl(int width, int height, bool michaelMode) {
        return new GLGame(width, height, michaelMode);
    }

    void destroy_gui_gl(void* gui) {
        delete reinterpret_cast<GLGame*>(gui);
    }

    void display_gui_gl(void* gui, const Game& game) {
        reinterpret_cast<GLGame*>(gui)->display(game);
    }

    int input_gui_gl(void* gui) {
        return reinterpret_cast<GLGame*>(gui)->handleInput();
    }
}