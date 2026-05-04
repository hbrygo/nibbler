#include "../includes/game.hpp"
#include "../includes/nibbler.hpp"

class MLXGame {
    private:
        int _width, _height;

    public:
        MLXGame(int w, int h);
        ~MLXGame();
        void display(const Game& game);
        int handleInput();
};


MLXGame::MLXGame(int w, int h) : _width(w), _height(h) {
    // Initialize OpenMLX context and resources here
}

MLXGame::~MLXGame() {
    // Clean up OpenMLX resources here
}

void MLXGame::display(const Game& game) {
    (void)game; // To avoid unused parameter warning
    // Render the game state using OpenMLX here
}

int MLXGame::handleInput() {
    // Handle user input using OpenMLX here
    // Return the appropriate direction or mode based on input
    return 0; // Placeholder return value
}

extern "C" {
    void* create_gui_mlx(int width, int height) {
        return new MLXGame(width, height);
    }

    void destroy_gui_mlx(void* gui) {
        delete (MLXGame*)gui;
    }
    
    void display_gui_mlx(void* gui, const Game& game) {
        ((MLXGame*)gui)->display(game);
    }
    
    int input_gui_mlx(void* gui) {
        return ((MLXGame*)gui)->handleInput();
    }
}