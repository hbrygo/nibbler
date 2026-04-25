#include "nibbler.hpp"
#include <unistd.h>

GraphicsMode current_mode = THREE;

int main() {
	Game game(10, 10);
	game.displayGameArea();
	game.moveSnake();
	game.changeDirection(UP);
	while (1) {
		sleep(1);
		if (game.moveSnake() == -1) {
			break;
		}
		game.displayGameArea();
		switch (current_mode) {
			case ONE:
				std::cout << "Mode ONE active" << std::endl;
				break;
			case TWO:
				std::cout << "Mode TWO active" << std::endl;
				break;
			case THREE:
				std::cout << "Mode THREE active" << std::endl;
				break;
		}
		drawWindowThree();
	}
	return (0);
}
