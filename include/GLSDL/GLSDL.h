#pragma once
#ifdef NCH_GLSDL_OPENGL_BACKEND
#include <cstdint>
#include "GLSDL_render.h"
#include "GLSDL_shader.h"
#include "GLSDL_video.h"

class GLSDL {
public:
    static int init(uint32_t sdlFlags);
    static void quit();
private:
};

int GLSDL_Init(uint32_t sdlFlags);
void GLSDL_Quit();

#else
#include "GLSDL_SDL_defs.h"
#endif