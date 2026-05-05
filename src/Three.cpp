#include "nibbler.hpp"
#include <dlfcn.h>
#include <map>

static void *gl_handle = nullptr;
static void *glfw_handle = nullptr;
static GLFWwindow *window = nullptr;
static bool initialized = false;

extern GraphicsMode current_mode;

#define GLFW_PRESS 1
#define GLFW_RELEASE 0
#define GLFW_KEY_1 49
#define GLFW_KEY_2 50
#define GLFW_KEY_3 51
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_RIGHT 262

static std::map<int, bool> key_states;
static std::map<int, bool> key_pressed_this_frame;

typedef void (*PFNGLCLEARCOLORPROC)(float, float, float, float);
typedef void (*PFNGLCLEARPROC)(unsigned int);
typedef void (*PFNGLFLUSHPROC)(void);

static PFNGLCLEARCOLORPROC glClearColor_ptr = nullptr;
static PFNGLCLEARPROC glClear_ptr = nullptr;
static PFNGLFLUSHPROC glFlush_ptr = nullptr;

typedef int (*PFNGLFWINITPROC)(void);
typedef GLFWwindow* (*PFNGLFWCREATEWINDOWPROC)(int, int, const char*, void*, void*);
typedef void (*PFNGLFWMAKECONTEXTCURRENTPROC)(GLFWwindow*);
typedef void (*PFNGLFWSWAPINTERVALPROC)(int);
typedef int (*PFNGLFWWINDOWSHOULDCLOSEPROC)(GLFWwindow*);
typedef void (*PFNGLFWDESTROYWINDOWPROC)(GLFWwindow*);
typedef void (*PFNGLFWTERMINATEPROC)(void);
typedef void (*PFNGLFWSWAPBUFFERSPROC)(GLFWwindow*);
typedef void (*PFNGLFWPOLLEVENTSPROC)(void);
typedef void (*PFNGLFWSETKEYPROC)(GLFWwindow*, int, int, int, int);

static PFNGLFWINITPROC glfwInit_ptr = nullptr;
static PFNGLFWCREATEWINDOWPROC glfwCreateWindow_ptr = nullptr;
static PFNGLFWMAKECONTEXTCURRENTPROC glfwMakeContextCurrent_ptr = nullptr;
static PFNGLFWSWAPINTERVALPROC glfwSwapInterval_ptr = nullptr;
static PFNGLFWWINDOWSHOULDCLOSEPROC glfwWindowShouldClose_ptr = nullptr;
static PFNGLFWDESTROYWINDOWPROC glfwDestroyWindow_ptr = nullptr;
static PFNGLFWTERMINATEPROC glfwTerminate_ptr = nullptr;
static PFNGLFWSWAPBUFFERSPROC glfwSwapBuffers_ptr = nullptr;
static PFNGLFWPOLLEVENTSPROC glfwPollEvents_ptr = nullptr;
typedef void (*PFNGLFWSETKEYCALLBACKPROC)(GLFWwindow*, PFNGLFWSETKEYPROC);
static PFNGLFWSETKEYCALLBACKPROC glfwSetKeyCallback_ptr = nullptr;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;
    
    if (action == GLFW_PRESS) {
        key_states[key] = true;
        key_pressed_this_frame[key] = true;
    } else if (action == GLFW_RELEASE) {
        key_states[key] = false;
    }
}

void checkKeyInputs()
{
    if (key_pressed_this_frame[GLFW_KEY_1]) {
        current_mode = ONE;
    }
    if (key_pressed_this_frame[GLFW_KEY_2]) {
        current_mode = TWO;
    }
    if (key_pressed_this_frame[GLFW_KEY_3]) {
        current_mode = THREE;
    }
    if (key_pressed_this_frame[GLFW_KEY_LEFT])
        std::cout << "Left key pressed" << std::endl;
    if (key_pressed_this_frame[GLFW_KEY_RIGHT])
        std::cout << "Right key pressed" << std::endl;
    
    key_pressed_this_frame.clear();
}

void drawWindowThree()
{
    if (!initialized)
    {
        gl_handle = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
        if (!gl_handle) {
            gl_handle = dlopen("libGL.so", RTLD_LAZY | RTLD_GLOBAL);
        }
        if (!gl_handle) {
            gl_handle = dlopen("/usr/lib/x86_64-linux-gnu/libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
        }
        if (!gl_handle) {
            std::cerr << "Erreur: Impossible de charger libGL.so" << std::endl;
            return;
        }
        
        glfw_handle = dlopen("./glfw-3.4/build/src/libglfw.so", RTLD_LAZY);
        if (!glfw_handle) {
            std::cerr << "Erreur: Impossible de charger libglfw.so - " << dlerror() << std::endl;
            dlclose(gl_handle);
            return;
        }
        
        glfwInit_ptr = (PFNGLFWINITPROC)dlsym(glfw_handle, "glfwInit");
        glfwCreateWindow_ptr = (PFNGLFWCREATEWINDOWPROC)dlsym(glfw_handle, "glfwCreateWindow");
        glfwMakeContextCurrent_ptr = (PFNGLFWMAKECONTEXTCURRENTPROC)dlsym(glfw_handle, "glfwMakeContextCurrent");
        glfwSwapInterval_ptr = (PFNGLFWSWAPINTERVALPROC)dlsym(glfw_handle, "glfwSwapInterval");
        glfwWindowShouldClose_ptr = (PFNGLFWWINDOWSHOULDCLOSEPROC)dlsym(glfw_handle, "glfwWindowShouldClose");
        glfwDestroyWindow_ptr = (PFNGLFWDESTROYWINDOWPROC)dlsym(glfw_handle, "glfwDestroyWindow");
        glfwTerminate_ptr = (PFNGLFWTERMINATEPROC)dlsym(glfw_handle, "glfwTerminate");
        glfwSwapBuffers_ptr = (PFNGLFWSWAPBUFFERSPROC)dlsym(glfw_handle, "glfwSwapBuffers");
        glfwPollEvents_ptr = (PFNGLFWPOLLEVENTSPROC)dlsym(glfw_handle, "glfwPollEvents");
        glfwSetKeyCallback_ptr = (PFNGLFWSETKEYCALLBACKPROC)dlsym(glfw_handle, "glfwSetKeyCallback");
        
        if (!glfwInit_ptr || !glfwCreateWindow_ptr || !glfwMakeContextCurrent_ptr || 
            !glfwSwapInterval_ptr || !glfwWindowShouldClose_ptr || !glfwDestroyWindow_ptr ||
            !glfwTerminate_ptr || !glfwSwapBuffers_ptr || !glfwPollEvents_ptr || !glfwSetKeyCallback_ptr) {
            std::cerr << "Erreur: Impossible de charger les symboles GLFW" << std::endl;
            dlclose(gl_handle);
            dlclose(glfw_handle);
            return;
        }
        
        if (!glfwInit_ptr()) {
            std::cerr << "Erreur: Impossible d'initialiser GLFW" << std::endl;
            dlclose(gl_handle);
            dlclose(glfw_handle);
            return;
        }
        
        window = glfwCreateWindow_ptr(100, 100, "Nibbler GL", nullptr, nullptr);
        if (!window) {
            std::cerr << "Erreur: Impossible de créer la fenêtre" << std::endl;
            glfwTerminate_ptr();
            dlclose(gl_handle);
            dlclose(glfw_handle);
            return;
        }
        
        glfwSetKeyCallback_ptr(window, key_callback);
        
        glfwMakeContextCurrent_ptr(window);
        glfwSwapInterval_ptr(1); // VSync
        
        glClearColor_ptr = (PFNGLCLEARCOLORPROC)dlsym(gl_handle, "glClearColor");
        glClear_ptr = (PFNGLCLEARPROC)dlsym(gl_handle, "glClear");
        glFlush_ptr = (PFNGLFLUSHPROC)dlsym(gl_handle, "glFlush");
        
        if (!glClearColor_ptr || !glClear_ptr || !glFlush_ptr) {
            std::cerr << "Erreur: Impossible de charger les symboles OpenGL" << std::endl;
            glfwDestroyWindow_ptr(window);
            glfwTerminate_ptr();
            dlclose(gl_handle);
            dlclose(glfw_handle);
            window = nullptr;
            return;
        }
        
        glClearColor_ptr(0.0f, 0.0f, 0.0f, 1.0f);
        initialized = true;
    }
    
    if (glfwWindowShouldClose_ptr(window)) {
        glfwDestroyWindow_ptr(window);
        glfwTerminate_ptr();
        dlclose(gl_handle);
        dlclose(glfw_handle);
        window = nullptr;
        gl_handle = nullptr;
        glfw_handle = nullptr;
        initialized = false;
        return;
    }
    
    glClear_ptr(0x4000); // GL_COLOR_BUFFER_BIT = 0x4000
    glFlush_ptr();
    
    glfwSwapBuffers_ptr(window);
    glfwPollEvents_ptr();
    checkKeyInputs();
}
