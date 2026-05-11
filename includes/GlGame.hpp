#ifndef GLGAME_HPP
# define GLGAME_HPP

# include "game.hpp"
# include "nibbler.hpp"
# include <algorithm>
# include <cmath>
# include <dlfcn.h>
# include <iostream>
# include <map>

struct GLFWwindow;

class GLGame
{
	private:
		static const int WINDOW_WIDTH = 1280;
		static const int WINDOW_HEIGHT = 720;

		GLFWwindow* _window;
		int _width;
		int _height;
		int _cell_size;
		Direction _lastDirection;

		void drawRect(float x, float y, float w, float h, float r, float g, float b) const;
		void drawQuad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
			float r, float g, float b) const;
		void drawLine(float x0, float y0, float x1, float y1, float r, float g, float b) const;

		static bool isSolidCell(const Game& game, int x, int y);

		void renderProjectedFloorGrid(double posX, double posY, double dirX, double dirY, int fbw, int fbh) const;
		static Direction turnLeft(Direction dir);
		static Direction turnRight(Direction dir);
		void renderRaycast(const Game& game, int fbw, int fbh) const;
		void renderMiniMap(const Game& game, int fbw, int fbh) const;

	public:
		GLGame(int w, int h);
		GLGame(const GLGame& other) = delete;
		GLGame& operator=(const GLGame& other) = delete;
		~GLGame();

		void display(const Game& game);
		int handleInput();
		bool isReady() const;
};

#endif
