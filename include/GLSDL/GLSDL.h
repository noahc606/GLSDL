#pragma once
#include <cstdint>
#include "GLSDL_pixels.h"
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