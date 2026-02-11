#pragma once
#ifdef NCH_GLSDL_OPENGL_BACKEND
#include <SDL2/SDL_video.h>
#include <cstdint>
#include <string>

class GLSDL_Window {
public:
    GLSDL_Window(SDL_Window* sdlwindow);
    SDL_Window* toSDL_Window();

private:
    void* vpSDL_Window = nullptr;
};

GLSDL_Window* GLSDL_CreateWindow(const std::string& title, int x, int y, int w, int h, uint32_t flags);
#endif