#include "nibbler.hpp"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

static GLFWwindow *g_window_gl = nullptr;

// ...existing code...
void AgraphiqueGL::init()
{
    if (!glfwInit())
        return;
    g_window_gl = glfwCreateWindow(800, 600, "Nibbler - GL (red)", nullptr, nullptr);
    if (!g_window_gl)
    {
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(g_window_gl);
    glfwSwapInterval(1); // vsync
}

// ...existing code...
void AgraphiqueGL::draw()
{
    if (!g_window_gl)
        return;
    int w, h;
    glfwGetFramebufferSize(g_window_gl, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(g_window_gl);
    // poll events so window closes properly elsewhere
    glfwPollEvents();
}

// ...existing code...
Direction AgraphiqueGL::getInput()
{
    if (g_window_gl && glfwWindowShouldClose(g_window_gl))
    {
        // map to your Direction::QUIT here if defined
    }
    // no mapping implemented yet
    return Direction::NONE;
}

// ...existing code...
void AgraphiqueGL::cleanup()
{
    if (g_window_gl)
    {
        glfwDestroyWindow(g_window_gl);
        g_window_gl = nullptr;
    }
    glfwTerminate();
}