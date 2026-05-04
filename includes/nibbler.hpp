#ifndef NIBBLER_HPP
# define NIBBLER_HPP

# include <iostream>
# include "game.hpp"
# include "Agraphique.hpp"
# include "graphiqueGL.hpp"
# include "graphiqueMLX.hpp"
# include <GLFW/glfw3.h>

enum GraphicLibrary {
    MLX = 1,
    SDL3 = 2,
    GL = 3
};

extern int currentLibrary;

#endif