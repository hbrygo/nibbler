#include "nibbler.hpp"
#include <unistd.h>

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
	}
	return (0);
}
