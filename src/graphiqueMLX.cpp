// ...existing code...
#include "nibbler.hpp"
#include <SDL2/SDL.h>

static SDL_Window *g_win = nullptr;
static SDL_Renderer *g_renderer = nullptr;
// ...existing code...
void AgraphiqueMLX::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        // SDL_Init failed, could log SDL_GetError() if souhaité
        return;
    }
    g_win = SDL_CreateWindow("Nibbler - SDL2 (blue)",
                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             800, 600, SDL_WINDOW_SHOWN);
    if (!g_win)
    {
        // failed to create window
        SDL_Quit();
        return;
    }
    g_renderer = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer)
    {
        SDL_DestroyWindow(g_win);
        g_win = nullptr;
        SDL_Quit();
        return;
    }
}
// ...existing code...
void AgraphiqueMLX::draw()
{
    if (!g_renderer)
        return;

    // Fill the screen with blue more efficiently than per-pixel drawing
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 255, 255);
    SDL_RenderClear(g_renderer);

    // Si tu veux dessiner des éléments supplémentaires, fais-le ici
    SDL_RenderPresent(g_renderer);
    // No blocking loop here
}
// ...existing code...
Direction AgraphiqueMLX::getInput()
{
    if (!g_win)
        return Direction::NONE;

    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        if (ev.type == SDL_QUIT)
            return Direction::QUIT;

        if (ev.type == SDL_KEYDOWN)
        {
            switch (ev.key.keysym.sym)
            {
            case SDLK_UP:
                return Direction::UP;
            case SDLK_DOWN:
                return Direction::DOWN;
            case SDLK_LEFT:
                return Direction::LEFT;
            case SDLK_RIGHT:
                return Direction::RIGHT;
            default:
                break;
            }
        }
    }
    return Direction::NONE;
}
// ...existing code...
void AgraphiqueMLX::cleanup()
{
    if (g_renderer)
    {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = nullptr;
    }
    if (g_win)
    {
        SDL_DestroyWindow(g_win);
        g_win = nullptr;
    }
    SDL_Quit();
}
// ...existing code...