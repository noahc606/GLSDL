#include "GLSDL.h"
#include <nch/cpp-utils/log.h>
#include <SDL2/SDL.h>
using namespace nch;

int GLSDL::init(uint32_t sdlFlags)
{
    int ret = SDL_Init(sdlFlags);

#if NCH_GLSDL_OPENGL_BACKEND>=1
    /* Set SDL-GL Attributes before creating window */
    {
        //Version
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
        //Set the red, green, and blue sizes to 8 bits each (24-bit color mode)
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        //Set the depth buffer size (optional, used if you're using 3D rendering)
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        //Set double buffering (optional, helps with smooth rendering)
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        if(ret==0) {
            Log::log("Successfully initialized GLSDL OpenGL backend.");
        } else {
            Log::log("Failed to initialize GLSDL OpenGL backend.");
        }
    }
#endif
    return ret;
}
void GLSDL::quit() {
    SDL_Quit();
}

int GLSDL_Init(uint32_t sdlFlags) {
    return GLSDL::init(sdlFlags);
}
void GLSDL_Quit() {
    return GLSDL::quit();
}