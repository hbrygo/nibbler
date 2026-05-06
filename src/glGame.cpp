#include "../includes/game.hpp"
#include "../includes/nibbler.hpp"

#include <algorithm>
#include <dlfcn.h>
#include <iostream>
#include <map>

#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_PROJECTION 0x1701
#define GL_MODELVIEW 0x1700
#define GL_QUADS 0x0007

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
        !glfwSetKeyCallback_ptr || !glfwGetFramebufferSize_ptr || !glClearColor_ptr || !glClear_ptr ||
        !glViewport_ptr || !glMatrixMode_ptr || !glLoadIdentity_ptr || !glOrtho_ptr ||
        !glBegin_ptr || !glEnd_ptr || !glColor3f_ptr || !glVertex2f_ptr) {
        std::cerr << "[GL] Missing required OpenGL/GLFW symbols" << std::endl;
        unload_symbols();
        return false;
    }

    symbols_loaded = true;
    return true;
}

class GLGame {
    private:
        GLFWwindow* _window;
        int _width;
        int _height;
        int _cell_size;

        void drawRect(float x, float y, float w, float h, float r, float g, float b) const {
            glColor3f_ptr(r, g, b);
            glBegin_ptr(GL_QUADS);
            glVertex2f_ptr(x, y);
            glVertex2f_ptr(x + w, y);
            glVertex2f_ptr(x + w, y + h);
            glVertex2f_ptr(x, y + h);
            glEnd_ptr();
        }

    public:
        GLGame(int w, int h);
        ~GLGame();
        void display(const Game& game);
        int handleInput();
        bool isReady() const;
};

GLGame::GLGame(int w, int h)
    : _window(nullptr), _width(std::max(10, w)), _height(std::max(10, h)), _cell_size(32) {
    if (!load_glfw_gl_symbols()) {
        return;
    }

    if (!glfw_initialized) {
        if (!glfwInit_ptr()) {
            std::cerr << "[GL] glfwInit failed" << std::endl;
            return;
        }
        glfw_initialized = true;
    }

    const int pixel_width = std::max(400, _width * _cell_size);
    const int pixel_height = std::max(400, _height * _cell_size);
    _window = glfwCreateWindow_ptr(pixel_width, pixel_height, "Nibbler - OpenGL", nullptr, nullptr);
    if (!_window) {
        std::cerr << "[GL] glfwCreateWindow failed" << std::endl;
        return;
    }

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

    const float cell_w = static_cast<float>(fbw) / static_cast<float>(_width);
    const float cell_h = static_cast<float>(fbh) / static_cast<float>(_height);

    for (int y = 0; y < _height; ++y) {
        for (int x = 0; x < _width; ++x) {
            const float px = static_cast<float>(x) * cell_w;
            const float py = static_cast<float>(y) * cell_h;

            drawRect(px, py, cell_w - 1.0f, cell_h - 1.0f, 0.15f, 0.16f, 0.18f);

            const int cell = game.getCell(x, y);
            if (cell == FOOD) {
                drawRect(px + 4.0f, py + 4.0f, cell_w - 8.0f, cell_h - 8.0f, 0.90f, 0.20f, 0.16f);
            } else if (cell == SNAKE) {
                drawRect(px + 2.0f, py + 2.0f, cell_w - 4.0f, cell_h - 4.0f, 0.16f, 0.78f, 0.25f);
            } else if (cell == WALL) {
                drawRect(px + 2.0f, py + 2.0f, cell_w - 4.0f, cell_h - 4.0f, 0.60f, 0.60f, 0.60f);
            }
        }
    }

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
        currentLibrary = SFML;
        return 10;
    }
    if (edge_pressed[GLFW_KEY_2]) {
        edge_pressed.clear();
        currentLibrary = SDL3;
        return 20;
    }
    if (edge_pressed[GLFW_KEY_3]) {
        edge_pressed.clear();
        currentLibrary = GL;
        return 30;
    }
    if (edge_pressed[GLFW_KEY_UP]) {
        edge_pressed.clear();
        return UP;
    }
    if (edge_pressed[GLFW_KEY_DOWN]) {
        edge_pressed.clear();
        return DOWN;
    }
    if (edge_pressed[GLFW_KEY_LEFT]) {
        edge_pressed.clear();
        return LEFT;
    }
    if (edge_pressed[GLFW_KEY_RIGHT]) {
        edge_pressed.clear();
        return RIGHT;
    }

    edge_pressed.clear();
    return 0;
}

extern "C" {
    void* create_gui_gl(int width, int height) {
        return new GLGame(width, height);
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