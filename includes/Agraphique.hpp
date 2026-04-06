#ifndef AGRAPHIQUE_HPP
# define AGRAPHIQUE_HPP

#include "nibbler.hpp"

class Agraphique
{
    public:
        virtual void init() = 0;
        virtual void draw() = 0;
        virtual Direction getInput() = 0;
        virtual void cleanup() = 0;
        virtual ~Agraphique() {}
};

#endif