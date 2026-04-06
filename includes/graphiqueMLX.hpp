#ifndef AGRAPHIQUEMLX_HPP
# define AGRAPHIQUEMLX_HPP

# include "nibbler.hpp"

class AgraphiqueMLX : public Agraphique
{
public:
    void init() override;
    void draw() override;
    Direction getInput() override;
    void cleanup() override;
};

#endif
