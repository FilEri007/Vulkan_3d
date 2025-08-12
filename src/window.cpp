#include <iostream>
#include <stdexcept>

#include <SDL3/SDL.h>

#include "window.hpp"

Window::Window(const char *title, unsigned int width, unsigned int height, SDL_WindowFlags flags)
{
    sdl_window = SDL_CreateWindow(title, width, height, flags);
    if (!sdl_window)
    {
        throw std::runtime_error(SDL_GetError());
    }
}

const SDL_Window *Window::get_sdl_window(void)
{
    return sdl_window;
}

Window::~Window()
{
    if (sdl_window)
    {
        SDL_DestroyWindow(sdl_window);
    }
}