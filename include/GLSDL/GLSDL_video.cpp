#include "GLSDL_video.h"
#ifdef NCH_GLSDL_OPENGL_BACKEND
#include <SDL2/SDL.h>

GLSDL_Window::GLSDL_Window(SDL_Window* sdlwindow) {
    GLSDL_Window::vpSDL_Window = sdlwindow;
}
SDL_Window* GLSDL_Window::toSDL_Window() {
    return (SDL_Window*)vpSDL_Window;
}

GLSDL_Window* GLSDL_CreateWindow(const std::string& title, int x, int y, int w, int h, uint32_t flags)
{
    uint32_t flagsMod = flags;
#if NCH_GLSDL_OPENGL_BACKEND>=1
    flags = flags | SDL_WINDOW_OPENGL;
#endif

    SDL_Window* sdlWin = SDL_CreateWindow(title.c_str(), x, y, w, h, flagsMod);

    GLSDL_Window* ret = new GLSDL_Window(sdlWin);
    if(ret->toSDL_Window()==nullptr) {
        delete ret; ret = nullptr;
    }

    return ret;
}

#endif