#include "Main.h"
#include <GL/glew.h>
#include <GLSDL/GLSDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_blendmode.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <nch/cpp-utils/log.h>
#include <nch/cpp-utils/timer.h>
#include <nch/sdl-utils/main-loop-driver.h>
#include <nch/sdl-utils/input.h>
#include <nch/sdl-utils/rect.h>
#include <nch/sdl-utils/text.h>
#include <nch/sdl-utils/texture-utils.h>

using namespace nch;

std::string basePath = "";
GLSDL_Renderer* glsdlRend = nullptr;
GLSDL_Window* glsdlWin = nullptr;
GLSDL_Shader* sdrDefault2D_Tex = nullptr, *sdrDefault3D = nullptr;
TTF_Font* bteFont = nullptr;
GLSDL_Texture* texC = nullptr;
GLSDL_Texture* texSquare0 = nullptr;
GLSDL_Texture* texSquare1 = nullptr;

SDL_Vertex rtGeoVerts[4] = {
    { {  5, 30 }, { 255,   0,   0, 255 }, { 0, 0 }},
    { { 27, 30 }, {   0, 255,   0, 255 }, { 1, 0 }},
    { { 32,  2 }, {   0,   0, 255, 255 }, { 1, 1 }},
    { {  5,  2 }, { 255, 255, 255,   0 }, { 0, 1 }}
};
SDL_Vertex geoVerts[4] = {
    { { 300, 400 }, { 255,   0,   0, 255 }, { 0, 0 }},
    { { 600, 400 }, {   0, 255,   0, 255 }, { 1, 0 }},
    { { 800, 100 }, {   0,   0, 255, 255 }, { 1, 1 }},
    { { 300, 100 }, { 255, 255, 255,   0 }, { 0, 1 }}
};
int geoNumVerts = sizeof(geoVerts) / sizeof(geoVerts[0]);
int geoInds[6] = { 0, 1, 2, 2, 3, 0 }; // Two triangles to form the rectangle
int geoNumInds = sizeof(geoInds) / sizeof(geoInds[0]);

bool paused = false;
bool clipping = false;

int64_t numTicks = 0;

int Main::getWidth() {
    int ret; SDL_GetWindowSize(glsdlWin->toSDL_Window(), &ret, NULL); return ret;
}
int Main::getHeight() {
    int ret; SDL_GetWindowSize(glsdlWin->toSDL_Window(), NULL, &ret); return ret;
}
std::string Main::getBasePath() {
    return basePath;
}

Main::Main()
{
    GLSDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    char* bp = SDL_GetBasePath();
    basePath = bp;
    SDL_free(bp);
    

    std::string title = "Raw SDL Renderer";
    #if NCH_GLSDL_OPENGL_BACKEND>=1
        title = "GL Implementation of SDL Renderer";
    #endif
    glsdlWin = GLSDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,  SDL_WINDOW_RESIZABLE);
    glsdlRend = GLSDL_CreateRenderer(glsdlWin, -1, SDL_RENDERER_ACCELERATED);
    if(glsdlRend==nullptr) {
        Log::errorv(__PRETTY_FUNCTION__, "GLSDL_CreateRenderer", "Failed to create GLSDL_Renderer.");
    }

    bteFont = TTF_OpenFont((basePath+"/BackToEarth.ttf").c_str(), 100);
    
    GLSDL_GL_SaveState(glsdlRend);
    {
        auto img = IMG_Load((basePath+"/test.png").c_str());
        SDL_PixelFormatEnum surfPxFmt = SDL_PIXELFORMAT_RGBA8888;
        SDL_Surface* surf = SDL_ConvertSurfaceFormat(img, surfPxFmt, 0);
        SDL_FreeSurface(img);
        texSquare0 = GLSDL_CreateTexture(glsdlRend, surfPxFmt, SDL_TEXTUREACCESS_STREAMING, surf->w, surf->h);
        GLSDL_UpdateTexture(texSquare0, NULL, surf->pixels, surf->pitch);
        {
            SDL_Rect uRect = {8, 8, 16, 16};
            int pitch;
            void* pixels;
            GLSDL_LockTexture(texSquare0, &uRect, &pixels, &pitch);
            TexUtils::setPixelColor(pixels, pitch, 4, 0, 0, Color(0, 127, 255).getRGBA());
            TexUtils::setPixelColor(pixels, pitch, 4, 1, 1, Color(255, 127, 0).getRGBA());
            TexUtils::setPixelColor(pixels, pitch, 4, 0, 2, Color(255, 0, 0).getRGBA());
            TexUtils::setPixelColor(pixels, pitch, 4, 1, 3, Color(0, 255, 0).getRGBA());
            TexUtils::setPixelColor(pixels, pitch, 4, 2, 4, Color(0, 0, 255).getRGBA());
            GLSDL_UnlockTexture(texSquare0);
        }
        SDL_FreeSurface(surf);
        
        auto imgC = IMG_Load((basePath+"/clippy.png").c_str());
        SDL_Surface* surfC = SDL_ConvertSurfaceFormat(imgC, SDL_PIXELFORMAT_RGBA8888, 0);
        SDL_FreeSurface(imgC);
        texC = GLSDL_CreateTextureFromSurface(glsdlRend, surfC);
        SDL_FreeSurface(surfC);

        texSquare1 = GLSDL_CreateTexture(glsdlRend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 64, 64);
        int ret = GLSDL_SetRenderTarget(glsdlRend, texSquare1); {
            SDL_Rect dst0 = {0, 0, 64, 64};
            SDL_Rect vp, er = {0, 0, 0, 0};
            
            //Background
            GLSDL_RenderSetViewport(glsdlRend, NULL);
            GLSDL_RenderGetViewport(glsdlRend, &vp);
            GLSDL_RenderCopy(glsdlRend, texSquare0, NULL, NULL);
            
            //Red top left
            dst0 = {1, 1, 30, 30};
            GLSDL_RenderSetViewport(glsdlRend, &dst0);
            GLSDL_RenderGetViewport(glsdlRend, &vp);              Log::log("VP-tl: %d %d %d %d", dst0.x, dst0.y, dst0.w, dst0.h);
            try { er = glsdlRend->getEffectiveRenderTargetRect(); Log::log("ER-tl: %d %d %d %d", dst0.x, dst0.y, dst0.w, dst0.h); } catch(...){}
            GLSDL_SetRenderDrawColor(glsdlRend, 255, 0, 0, 255);
            GLSDL_RenderFillRect(glsdlRend, NULL);
            //Blue top right
            dst0 = {33, 1, 30, 30};
            GLSDL_RenderSetViewport(glsdlRend, &dst0);
            GLSDL_RenderGetViewport(glsdlRend, &vp);              Log::log("VP-tr: %d %d %d %d", dst0.x, dst0.y, dst0.w, dst0.h);
            try { er = glsdlRend->getEffectiveRenderTargetRect(); Log::log("ER-tr: %d %d %d %d", dst0.x, dst0.y, dst0.w, dst0.h); } catch(...){}
            GLSDL_SetRenderDrawColor(glsdlRend, 0, 0, 191, 255);
            GLSDL_RenderFillRect(glsdlRend, NULL);
            //Yellow bottom left
            dst0 = {1, 33, 30, 30};
            GLSDL_RenderSetViewport(glsdlRend, &dst0);
            GLSDL_RenderGetViewport(glsdlRend, &vp);              Log::log("VP-bl: %d %d %d %d", dst0.x, dst0.y, dst0.w, dst0.h);
            try { er = glsdlRend->getEffectiveRenderTargetRect(); Log::log("ER-bl: %d %d %d %d", dst0.x, dst0.y, dst0.w, dst0.h); } catch(...){}
            GLSDL_SetRenderDrawColor(glsdlRend, 255, 255, 0, 255);
            GLSDL_RenderFillRect(glsdlRend, NULL);
            //Geometry bottom right
            dst0 = {33, 33, 30, 30};
            GLSDL_RenderSetViewport(glsdlRend, &dst0);
            GLSDL_RenderGetViewport(glsdlRend, &vp);              Log::log("VP-br: %d %d %d %d", dst0.x, dst0.y, dst0.w, dst0.h);
            try { er = glsdlRend->getEffectiveRenderTargetRect(); Log::log("ER-br: %d %d %d %d", dst0.x, dst0.y, dst0.w, dst0.h); } catch(...){}
            GLSDL_RenderGeometry(glsdlRend, texSquare0, rtGeoVerts, geoNumVerts, geoInds, geoNumInds);        

            GLSDL_RenderSetViewport(glsdlRend, NULL);

            SDL_Surface* temp = SDL_CreateRGBSurfaceWithFormat(0, 64, 64, 32, SDL_PIXELFORMAT_RGBA8888);
            {
                Timer tim("RenderReadPixels", true);
                GLSDL_RenderReadPixels(glsdlRend, NULL, SDL_PIXELFORMAT_RGBA8888, temp->pixels, temp->pitch);
            }

            std::string filename = basePath+"/gen_background_sdl.png";
        #if NCH_GLSDL_OPENGL_BACKEND>=1
            filename = basePath+"/gen_background_glsdl.png";
        #endif
            IMG_SavePNG(temp, filename.c_str());
        } GLSDL_SetRenderTarget(glsdlRend, nullptr);
    }
    GLSDL_GL_RestoreState(glsdlRend);

    MainLoopDriver mld(tick, 50, draw, 1000, events);
}
Main::~Main()
{
    delete sdrDefault2D_Tex;
    delete sdrDefault3D;
}
int main(int argc, char** args)
{ Main m; return 0; }

void Main::tick() {
    if(!paused) {
        numTicks++;
    }

    if(Input::keyDownTime(SDLK_SPACE)==1) {
        numTicks++;
    }
    if(Input::keyDownTime(SDLK_c)==1) {
        clipping = !clipping;
    }
    if(Input::keyDownTime(SDLK_ESCAPE)==1) {
        paused = !paused;
    }
    if(Input::keyDownTime(SDLK_s)==1) {
        Timer tim("RenderReadPixels screenshot", true);
        SDL_Surface* scrSurf = SDL_CreateRGBSurfaceWithFormat(0, getWidth(), getHeight(), 32, SDL_PIXELFORMAT_RGBA8888);
        GLSDL_RenderReadPixels(glsdlRend, NULL, scrSurf->format->format, scrSurf->pixels, scrSurf->pitch);
        std::string filename = basePath+"/gen_screenshot_sdl.png";
    #if NCH_GLSDL_OPENGL_BACKEND>=1
        filename = basePath+"/gen_screenshot_glsdl.png";
    #endif
        IMG_SavePNG(scrSurf, filename.c_str());
    }
}


void Main::draw() {
    /*
        Clear all rendering at the beginning of each frame
        Used functions: SetRenderDrawColor, RenderClear
    */
    GLSDL_GL_SaveState(glsdlRend);
    GLSDL_SetRenderDrawColor(glsdlRend, (numTicks/1)%256, (numTicks/2)%256, (numTicks/4)%256, 255);
    GLSDL_RenderClear(glsdlRend);
    GLSDL_GL_RestoreState(glsdlRend);
    
    /*
        Render 3D objects (world objects)
    */
    //draw3D();

    /*
        Render 2D objects (UIs, text, etc.)
        Used functions in init:
        - Tex-access STATIC:    CreateTextureFromSurface.
        - Tex-access STREAMING: CreateTexture(STREAMING); UpdateTexture; LockTexture; [setting lock pixels]; UnlockTexture.
        - Tex-access TARGET:    CreateTexture(TARGET); SetRenderTarget; RenderReadPixels (on render target).
        Used functions in draw loop:
        - Viewport/Cliprect:    RenderSetViewport; RenderSetClipRect; RenderIsClipEnabled; RenderGetClipRect.
        - Setting modes:        SetRenderDrawBlendMode; SetRenderDrawColor; SetTextureBlendMode; SetTextureColorMod; SetTextureAlphaMod.
        - Rendering RectF:      RenderFillRectF; RenderCopyF; RenderCopyExF.
        - Rendering misc:       RenderDrawLine(F); RenderDrawLine.
        - Querying:             QueryTexture; RenderReadPixels (on window).
    */
    GLSDL_GL_SaveState(glsdlRend);
    //Log::log("FrameStart...");
    draw2D();
    //Log::log("FrameFinished");
    GLSDL_GL_RestoreState(glsdlRend);
}

void Main::draw2D() {
    /* Find values for BlendMode and RendererFlip */
    SDL_BlendMode cycledBlendMode = SDL_BLENDMODE_NONE;
    SDL_RendererFlip cycledFlipMode = SDL_FLIP_NONE;
    {
        int blendModeID = (numTicks/50)%5;
        switch(blendModeID) {
            case 0: { cycledBlendMode = SDL_BLENDMODE_NONE; } break;
            case 1: { cycledBlendMode = SDL_BLENDMODE_BLEND; } break;
            case 2: { cycledBlendMode = SDL_BLENDMODE_ADD; } break;
            case 3: { cycledBlendMode = SDL_BLENDMODE_MOD; } break;
            case 4: { cycledBlendMode = SDL_BLENDMODE_MUL; } break;
        }
        int flipModeID = (numTicks/50)%3;
        switch(flipModeID) {
            case 0: { cycledFlipMode = SDL_FLIP_NONE; } break;
            case 1: { cycledFlipMode = SDL_FLIP_HORIZONTAL; } break;
            case 2: { cycledFlipMode = SDL_FLIP_VERTICAL; } break;
        }
    }

    /* Set Viewport */
    {
        SDL_Rect vp = {0, 0, Main::getWidth()/2, Main::getHeight()/2};
        switch(numTicks/25%16) {
            case 0: { } break;
            case 1: { vp.x += Main::getWidth()/2-1; } break;
            case 2: { vp.y += Main::getHeight()/2-1; } break;
            case 3: { vp.x += Main::getWidth()/2-1; vp.y += Main::getHeight()/2-1; } break;
            default: { vp.x = 0; vp.y = 0; vp.w = Main::getWidth(); vp.h = Main::getHeight(); }
        }
        GLSDL_RenderSetViewport(glsdlRend, &vp);
    }

    /* Set Clip Rect */
    if(clipping) {
        SDL_Rect clipRectDst = {0, 0, (int)(384+(numTicks%200-50)), (int)(384+(numTicks%200-50))};
        GLSDL_RenderSetClipRect(glsdlRend, &clipRectDst);
        assert(GLSDL_RenderIsClipEnabled(glsdlRend));
        SDL_Rect temp; GLSDL_RenderGetClipRect(glsdlRend, &temp);
        assert(Rect(temp)==Rect(clipRectDst));
    }

    GLSDL_SetRenderDrawBlendMode(glsdlRend, SDL_BLENDMODE_BLEND);
    GLSDL_SetRenderDrawColor(glsdlRend, 255, 255, 255, 255);
    GLSDL_RenderFillRectF(glsdlRend, NULL);
    //Destinations...
    SDL_FRect dst1 = {2, 2, Main::getWidth()-4.0f, Main::getHeight()-4.0f};
    dst1.x-=2; dst1.y-=2; dst1.w+=4; dst1.h+=4;
    SDL_FRect dst2 = {160, 160, 64, 64};
    SDL_FPoint fp = {150, 150};

    //RenderCopy a TEXTUREACCESS_TARGET texture which has been rendered to (purple and blue halves).
    if(true) {
        GLSDL_SetTextureBlendMode(texSquare1, cycledBlendMode);
        GLSDL_SetTextureColorMod(texSquare0, (numTicks/1)%256, (numTicks/2)%256, (numTicks/4)%256);
        GLSDL_SetTextureAlphaMod(texSquare1, 127);
        GLSDL_RenderCopyF(glsdlRend, texSquare1, NULL, &dst1);
        GLSDL_RenderCopyExF(glsdlRend, texSquare1, NULL, &dst1, 0, NULL, cycledFlipMode);
        GLSDL_SetRenderDrawColor(glsdlRend, 63, 0, 0, 255);
        GLSDL_RenderDrawRectF(glsdlRend, &dst1);
    }

    //RenderGeometry
    if(true) {
        for(int i = 0; i<geoNumVerts; i++) {
            geoVerts[i].color.a = 128+numTicks%128;
        }
        GLSDL_RenderGeometry(glsdlRend, texSquare0, geoVerts, geoNumVerts, geoInds, geoNumInds);        
    }


    //SetRenderDrawColor + RenderFillRect and RenderDrawRect
    if(true) {
        SDL_FRect rect = {60, 10, 100, 200};
        GLSDL_SetRenderDrawColor(glsdlRend, 255, 0, 0, 255);
        GLSDL_RenderFillRectF(glsdlRend, &rect);
        GLSDL_SetRenderDrawColor(glsdlRend, 0, 255, 0, 255);
        GLSDL_RenderDrawRectF(glsdlRend, &rect);        
    }

    //RenderDrawLine, RenderDrawPoint
    if(true) {
        GLSDL_RenderDrawLineF(glsdlRend, 12, 34, 56, 78);
        GLSDL_RenderDrawLine(glsdlRend, 0, 0, 100+numTicks/2, 200+numTicks);
        GLSDL_RenderDrawPoint(glsdlRend, 100, 125+numTicks/2);        
    }

    //RenderCopy a TEXTUREACCESS_STREAMING texture whose pixels were updated from an IMG_Loaded surface
    if(true) {
        GLSDL_SetTextureAlphaMod(texSquare0, 255);
        GLSDL_SetTextureBlendMode(texSquare0, SDL_BLENDMODE_BLEND);
        GLSDL_RenderCopyF(glsdlRend, texSquare0, NULL, &dst2);
        GLSDL_SetTextureAlphaMod(texSquare0, 175);
        GLSDL_RenderCopyExF(glsdlRend, texSquare0, NULL, &dst2, numTicks, &fp, cycledFlipMode);        
    }


    //If debugging and OpenGL backend: Visualize draw area resulting from RenderSetViewport and RenderSetClipRect
    if(false) {
        #if !NDEBUG
        #if NCH_GLSDL_OPENGL_BACKEND>=1
            SDL_Rect erRect = glsdlRend->getEffectiveRenderTargetRect();
            SDL_FRect erRectF = { (float)erRect.x+1, (float)erRect.y+1, (float)erRect.w-2, (float)erRect.h-2 };
            GLSDL_RenderSetViewport(glsdlRend, NULL);
            GLSDL_RenderSetClipRect(glsdlRend, NULL);
            GLSDL_SetRenderDrawColor(glsdlRend, 0, 63, 0, ((numTicks*2)%64)+64);
            GLSDL_RenderFillRectF(glsdlRend, &erRectF);
        #endif
        #endif
    }

    //RenderCopy a TEXTUREACCESS_STATIC texture whose pixels were updated from an IMG_Loaded surface
    if(true) {
        GLSDL_RenderSetViewport(glsdlRend, NULL);
        GLSDL_RenderSetClipRect(glsdlRend, NULL);
        //clippy
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if(numTicks%400>200) flip = SDL_FLIP_HORIZONTAL;

        int anim = numTicks%100;
        FRect dstc; {
            int wc, hc;
            GLSDL_QueryTexture(texC, NULL, NULL, &wc, &hc);
            wc /= 3; hc /= 3;
            dstc = FRect(getWidth()-wc, getHeight()-hc, wc, hc);

            if(numTicks/100%2) {
                dstc.r.x += dstc.r.w/2*anim/100;
                dstc.r.w -= dstc.r.w*anim/100;
            } else {
                dstc.r.x += dstc.r.w/2*(100-anim)/100;
                dstc.r.w -= dstc.r.w*(100-anim)/100;
            }
        }        

        GLSDL_RenderCopyExF(glsdlRend, texC, NULL, &dstc.r, 0, NULL, flip);
        GLSDL_RenderSetClipRect(glsdlRend, NULL);
    }

    GLSDL_RenderPresent(glsdlRend);
}
void Main::draw3D() {
    sdrDefault3D->useProgram();
    glEnable(GL_DEPTH_TEST);    //Depth testing
}

void Main::events(SDL_Event& evt) {

}
