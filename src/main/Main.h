#pragma once
#include <SDL2/SDL.h>
#include <string>

class Main {
public:
    Main(); ~Main();
    static int getWidth();
    static int getHeight();
    static std::string getBasePath();
private:
    static void tick();
    static void draw();
    static void draw2D();
    static void draw3D();
    static void events(SDL_Event& e);
};