#include "../includes/game.hpp"
#include "../includes/nibbler.hpp"

class GLGame {
    private:
        int _width, _height;

    public:
        GLGame(int w, int h);
        ~GLGame();
        void display(const Game& game);
        int handleInput();
};


GLGame::GLGame(int w, int h) : _width(w), _height(h) {
    // Initialize OpenGL context and resources here
}

GLGame::~GLGame() {
    // Clean up OpenGL resources here
}

void GLGame::display(const Game& game) {
    (void)game; // To avoid unused parameter warning
    // Render the game state using OpenGL here
}

int GLGame::handleInput() {
    // Handle user input using OpenGL here
    // Return the appropriate direction or mode based on input
    return 0; // Placeholder return value
}

extern "C" {
    void* create_gui_gl(int width, int height) {
        return new GLGame(width, height);
    }

    void destroy_gui_gl(void* gui) {
        delete (GLGame*)gui;
    }
    
    void display_gui_gl(void* gui, const Game& game) {
        ((GLGame*)gui)->display(game);
    }
    
    int input_gui_gl(void* gui) {
        return ((GLGame*)gui)->handleInput();
    }
}