#ifndef AGRAPHIQUEGL_HPP
# define AGRAPHIQUEGL_HPP

# include "nibbler.hpp"

class AgraphiqueGL : public Agraphique
{
public:
    void init() override;
    void draw() override;
    Direction getInput() override;
    void cleanup() override;
};

#endif
