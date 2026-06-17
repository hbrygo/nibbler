#ifndef INIBBLERDISPLAY_HPP
# define INIBBLERDISPLAY_HPP

# include "game.hpp"

class INibblerDisplay
{
public:
    virtual ~INibblerDisplay() {}

    virtual bool init(int width, int height, bool michaelMode) = 0;
    virtual int getEvents() = 0;
    virtual void updateGameData(const Game& game) = 0;
    virtual void refreshDisplay() = 0;
    virtual void stop() = 0;
};

#endif