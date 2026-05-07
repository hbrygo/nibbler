#include "../includes/game.hpp"
#include "../includes/nibbler.hpp"

class SFMLGame {
    private:
        int _width, _height;

    public:
        SFMLGame(int w, int h);
        ~SFMLGame();
        void display(const Game& game);
        int handleInput();
};


SFMLGame::SFMLGame(int w, int h) : _width(w), _height(h) {
    (void)_width;
    (void)_height;
    // Initialize OpenMLX context and resources here
}

SFMLGame::~SFMLGame() {
    // Clean up OpenMLX resources here
}

void SFMLGame::display(const Game& game) {
    (void)game; // To avoid unused parameter warning
    // Render the game state using OpenMLX here
}

int SFMLGame::handleInput() {
    // Handle user input using OpenMLX here
    // Return the appropriate direction or mode based on input
    return 0; // Placeholder return value
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