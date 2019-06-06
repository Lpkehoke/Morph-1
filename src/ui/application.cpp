#include "ui/application.h"

#include <SDL2/SDL.h>

#include <stdexcept>

namespace ui
{

application::application()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
    {
        throw std::runtime_error("Failed to initialize SDL");
    }
}

application::~application()
{
    SDL_Quit();
}

void application::render()
{
}

} // namespace ui
