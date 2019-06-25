#include "ui/application.h"

#include "vk/vkrenderer.h"

#include "foundation/vector.h" 

#include <SDL2/SDL.h>

#include <iostream>

#include <stdexcept>
#include <vector>
#include <utility>

namespace ui
{

Application::Application()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
    {
        throw std::runtime_error("Failed to initialize SDL");
    }
}

Application::~Application()
{
    SDL_Quit();
}

void Application::render()
{
    SDL_Event e;

    auto renderer = vk::RendererFactory::create();
    renderer->initialize();

    auto wnd = SDL_CreateWindow("Morph", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_VULKAN);
    auto ctx = SDL_GL_CreateContext(wnd);

    SDL_GL_SwapWindow(wnd);

    while (true)
    {
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT)
        {
            break;
        }
    }
}

} // namespace ui
