#include "GLSDL_render_impl.h"
#ifdef NCH_GLSDL_OPENGL_BACKEND
#include <SDL2/SDL_image.h>
#include <nch/cpp-utils/log.h>
#include <nch/math-utils/box2.h>
using namespace nch;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if NCH_GLSDL_OPENGL_BACKEND>=1
SDL_BlendMode GLSDL_Renderer::glCachedSDL_BlendMode = SDL_BLENDMODE_INVALID;
SDL_Rect GLSDL_Renderer::glCachedScissor = {0,0,-1,-1};
glm::mat4 GLSDL_Renderer::glCachedOrthoMat4 = glm::mat4(0.f);
int GLSDL_Renderer::glCachedApplyTexture = -1;

bool GLSDL_Renderer::prevGL_BlendEnabled = false;
GLint GLSDL_Renderer::prevGL_BlendSrcRGB = 0;
GLint GLSDL_Renderer::prevGL_BlendDstRGB = 0;
GLint GLSDL_Renderer::prevGL_BlendSrcAlpha = 0;
GLint GLSDL_Renderer::prevGL_BlendDstAlpha = 0;
GLint GLSDL_Renderer::prevGL_Viewport[4] = {0,0,0,0};
bool GLSDL_Renderer::prevGL_ScissorTestEnabled = false;
SDL_Rect GLSDL_Renderer::prevGL_ScissorBox = {0,0,0,0};
GLfloat GLSDL_Renderer::prevGL_ClearColor[4] = {0.f,0.f,0.f,0.f};
bool GLSDL_Renderer::prevGL_CullFaceEnabled = false;
bool GLSDL_Renderer::prevGL_DepthTestEnabled = false;
GLint GLSDL_Renderer::prevGL_ActiveTexture = GL_TEXTURE0;
GLint GLSDL_Renderer::prevGL_UsedProgram = 0;
#endif
bool GLSDL_Renderer::glSaveStateExists = false;

GLSDL_Renderer::GLSDL_Renderer(GLSDL_Window* window, int index, uint32_t flags)
{
    //Assign window
    if(!failedConstruction) {
        sdlWindow = window->toSDL_Window();
        if(window==nullptr) {
            failedConstruction = true;
        }
    }

    //Create SDL_Renderer
    if(!failedConstruction) {
        sdlRenderer = SDL_CreateRenderer(sdlWindow, index, flags);
        if(sdlRenderer==nullptr) {
            failedConstruction = true;
        }
    }

#if NCH_GLSDL_OPENGL_BACKEND>=1
    //Create GL_Context
    if(!failedConstruction) {
        sdlGlCtx = SDL_GL_CreateContext(sdlWindow);
        if(sdlGlCtx==nullptr) {
            failedConstruction = true;
        }
    }
    //Init GLEW
    if(!failedConstruction) {
        glewExperimental = GL_TRUE;  //Force GLEW to get exported functions instead of checking via extension string
        if(glewInit()!=GLEW_OK) {
            Log::errorv(__PRETTY_FUNCTION__, "glewInit()", "Failed to inititalize GLEW");
            failedConstruction = true;
        }
    } 
    //Create default 2D shader
    if(!failedConstruction) {
        sdrDefault2D = GLSDL_Shader::createDefault2D_TexOrSolid();
    }
#endif
}
GLSDL_Renderer::~GLSDL_Renderer() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(sdrDefault2D) delete sdrDefault2D;
    SDL_GL_DeleteContext(sdlGlCtx);
#endif
    SDL_DestroyRenderer(sdlRenderer);
}

bool GLSDL_Renderer::hasFailedConstruction() const {
    return failedConstruction;
}
SDL_Renderer* GLSDL_Renderer::toSDL_Renderer() const {
    return sdlRenderer;
}
SDL_GLContext GLSDL_Renderer::getSDL_GLContext() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return sdlGlCtx;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
int GLSDL_Renderer::getRenderDrawBlendMode(SDL_BlendMode* sdlBlendMode) const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(sdlBlendMode!=nullptr) *sdlBlendMode = renderDrawBlendMode;
    return 0;
#else
    return SDL_GetRenderDrawBlendMode(sdlRenderer, sdlBlendMode);
#endif
}
int GLSDL_Renderer::getRenderDrawColor(uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a) const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(r!=nullptr) *r = renderDrawColor.r*255.f;
    if(g!=nullptr) *g = renderDrawColor.g*255.f;
    if(b!=nullptr) *b = renderDrawColor.b*255.f;
    if(a!=nullptr) *a = renderDrawColor.a*255.f;
    return 0;
#else
    return SDL_GetRenderDrawColor(sdlRenderer, r, g, b, a);
#endif
}
void* GLSDL_Renderer::getRenderTarget() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return renderTarget;
#else
    return SDL_GetRenderTarget(sdlRenderer);
#endif
}
int GLSDL_Renderer::getRenderTargetW() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(renderTarget==nullptr) return windowW; 
    return reinterpret_cast<GLSDL_Texture*>(renderTarget)->getGL_TexWidth();
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
int GLSDL_Renderer::getRenderTargetH() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(renderTarget==nullptr) return windowH;
    return reinterpret_cast<GLSDL_Texture*>(renderTarget)->getGL_TexHeight();
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
SDL_Rect GLSDL_Renderer::getRenderTargetClipRect() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(!isRenderTargetClipEnabled()) {
        return {0, 0, getRenderTargetW(), getRenderTargetH()};
    }
    if(renderTarget==nullptr) { return ntgtRenderClipRect; }
    else                      { return reinterpret_cast<GLSDL_Texture*>(renderTarget)->getRenderClipRect(); }
#else
    SDL_Rect ret; SDL_RenderGetClipRect(sdlRenderer, &ret); return ret;
#endif
}
bool GLSDL_Renderer::isRenderTargetClipEnabled() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(renderTarget==nullptr) { return ntgtRenderClipEnabled; }
    else                      { return reinterpret_cast<GLSDL_Texture*>(renderTarget)->isRenderClipEnabled(); }
#else
    return SDL_RenderIsClipEnabled(sdlRenderer);
#endif
}
SDL_Rect GLSDL_Renderer::getRenderTargetViewport() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(!isRenderTargetViewportEnabled()) {
        return {0, 0, getRenderTargetW(), getRenderTargetH()};
    }
    if(renderTarget==nullptr) { return ntgtRenderViewport; }
    else                      { return reinterpret_cast<GLSDL_Texture*>(renderTarget)->getRenderViewport(); }
#else
    SDL_Rect ret;
    SDL_RenderGetClipRect(sdlRenderer, &ret);
    return ret;
#endif
}
bool GLSDL_Renderer::isRenderTargetViewportEnabled() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(renderTarget==nullptr) { return ntgtRenderViewportEnabled; }
    else                      { return reinterpret_cast<GLSDL_Texture*>(renderTarget)->isRenderViewportEnabled(); }
#else
    return SDL_RenderIsClipEnabled(sdlRenderer);
#endif
}
SDL_Rect GLSDL_Renderer::getEffectiveRenderTargetRect() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(renderTarget==nullptr) { return ntgtEffectiveRenderRect; }
    else                      { return reinterpret_cast<GLSDL_Texture*>(renderTarget)->getEffectiveRenderRect(); }
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::transformQuadVertsF(int rtH, GLfloat* verts, int stride, float angleDegrees, SDL_FPoint* center, SDL_RendererFlip flip) {
    SDL_FPoint cent;
    SDL_FPoint topLeft;
    topLeft.x = verts[0*stride+0];
    topLeft.y = verts[0*stride+1];
    for(int i = 0; i<4; i++) {
        if(verts[i*stride]<topLeft.x) {
            topLeft.x = verts[i*stride];
        }
        if(verts[i*stride+1]>topLeft.y) {
            topLeft.y = verts[i*stride+1];
        }
    }
    topLeft.y = rtH-topLeft.y;

    if(center) {
        cent = *center;
        cent.y = rtH-cent.y;
    } else {
        float centX = (verts[0*stride+0]+verts[1*stride+0]+verts[2*stride+0]+verts[3*stride+0])/4.0;
        float centY = (verts[0*stride+1]+verts[1*stride+1]+verts[2*stride+1]+verts[3*stride+1])/4.0;
        cent = {centX, centY};
    }
    
    float rad = -angleDegrees*0.01745329251994330f;
    float c = std::cos(rad);
    float s = std::sin(rad);
    for(int i = 0; i<4; i++) {
        int base = i*stride;
        //Read position
        float x = verts[base+0];
        float y = verts[base+1];
        //Translate to center
        x -= (cent.x+topLeft.x);
        y -= (cent.y-topLeft.y);

        //Apply rotation
        float xr = x*c-y*s;
        float yr = x*s+y*c;

        //Translate back
        verts[base+0] = xr+(cent.x+topLeft.x);
        verts[base+1] = yr+(cent.y-topLeft.y);
    }

    //Apply flip
    if(flip==SDL_FLIP_HORIZONTAL) {
        // 0 <-> 1 && 2 <-> 3
        std::swap(verts[0*stride+6], verts[1*stride+6]);
        std::swap(verts[2*stride+6], verts[3*stride+6]);
        std::swap(verts[0*stride+7], verts[1*stride+7]);
        std::swap(verts[2*stride+7], verts[3*stride+7]);
    } else if(flip==SDL_FLIP_VERTICAL) {
        // 0 <-> 3 && 1 <-> 2
        std::swap(verts[0*stride+6], verts[3*stride+6]);
        std::swap(verts[1*stride+6], verts[2*stride+6]);
        std::swap(verts[0*stride+7], verts[3*stride+7]);
        std::swap(verts[1*stride+7], verts[2*stride+7]);
    }
}
void GLSDL_Renderer::transformQuadVertsI(int rtH, float* verts, int stride, float angleDegrees, SDL_Point* center, SDL_RendererFlip flip) {
    if(center) {
        SDL_FPoint cent;
        cent.x = center->x;
        cent.y = center->y;
        transformQuadVertsF(rtH, verts, stride, angleDegrees, &cent, flip);
    } else {
        transformQuadVertsF(rtH, verts, stride, angleDegrees, nullptr, flip);
    }
}
void GLSDL_Renderer::translateVerts(void* renderTarget, const SDL_Rect& viewport, float* verts, int numVerts, int stride) {
    for(int i = 0; i<numVerts; i++) {
        verts[i*stride+0] += viewport.x;
        
        if(renderTarget) {
            verts[i*stride+1] += viewport.y;
            verts[i*stride+1] = reinterpret_cast<GLSDL_Texture*>(renderTarget)->getGL_TexHeight()-verts[i*stride+1];
        } else {
            verts[i*stride+1] -= viewport.y;
        }
    }
}
SDL_Rect GLSDL_Renderer::calcEffectiveRenderRect(int rtgtW, int rtgtH, const SDL_Rect& viewport, bool viewportEnabled, const SDL_Rect& clipRect, bool clipEnabled) {
    SDL_Rect vp = viewport;
    if(!viewportEnabled) {
        vp = {0, 0, rtgtW, rtgtH};
    }
    SDL_Rect cr = clipRect;
    if(!clipEnabled) {
        cr = {0, 0, rtgtW, rtgtH};
    }

    Box2i retBox = Box2i::createFromXYWH(vp.x, vp.y, vp.w, vp.h)
        .intersection(Box2i::createFromXYWH(vp.x+cr.x, vp.y+cr.y, cr.w, cr.h));
    return { retBox.c1.x, retBox.c1.y, retBox.getWidth(), retBox.getHeight() };
}

void GLSDL_Renderer::glSaveState(GLSDL_Renderer* glsdlRenderer) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(glSaveStateExists) {
        throw std::logic_error("GLSDL_GL_SaveState called twice in a row - call GLSDL_GL_RestoreState before calling GLSDL_GL_SaveState again");
    }
    if(glsdlRenderer==nullptr) {
        throw std::logic_error("glsdlRenderer*==nullptr");
    }

    saveGL_BlendFuncState();
    saveGL_ScissorState();
    saveGL_CullFaceState();
    saveGL_DepthTestState();
    saveGL_ActiveTextureState();
    saveGL_UsedProgram();

    glEnable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glsdlRenderer->sdrDefault2D->useProgram();
    glsdlRenderer->vpUpdate();

    glCachedSDL_BlendMode = SDL_BLENDMODE_INVALID;
    glCachedScissor = {0,0,-1,-1};
    glCachedOrthoMat4 = glm::mat4(0.f);
    glCachedApplyTexture = -1;
    glSaveStateExists = true;
#endif
}
void GLSDL_Renderer::assertGL_SaveStateExists() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
#ifndef NDEBUG
    if(!glSaveStateExists) {
        Log::error(__PRETTY_FUNCTION__, "Function should have been called between GLSDL_GL_SaveState() and GLSDL_GL_RestoreState()");
        std::abort();
    }
#endif
#endif
}
void GLSDL_Renderer::glRestoreState() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(!glSaveStateExists) {
        throw std::logic_error("GLSDL_GL_RestoreState called incorrectly - call GLSDL_GL_SaveState first");
    }
    restoreGL_BlendFuncState();
    restoreGL_ScissorState();
    restoreGL_CullFaceState();
    restoreGL_DepthTestState();
    restoreGL_ActiveTextureState();
    restoreGL_UsedProgram();

    glCachedSDL_BlendMode = SDL_BLENDMODE_INVALID;
    glCachedScissor = {0,0,-1,-1};
    glCachedOrthoMat4 = glm::mat4(0.f);
    glCachedApplyTexture = -1;
    glSaveStateExists = false;
#endif
}

int GLSDL_Renderer::renderClear() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    glClearColor(renderDrawColor.r, renderDrawColor.g, renderDrawColor.b, renderDrawColor.a); //Set clear color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear
    return 0;
#else
    return SDL_RenderClear(sdlRenderer);
#endif
}
void GLSDL_Renderer::renderPresent() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    SDL_GL_SwapWindow(sdlWindow);
#else
    SDL_RenderPresent(sdlRenderer);
#endif
    
}
int GLSDL_Renderer::renderReadPixels(const SDL_Rect* rect, uint32_t format, void* pixels, int pitch) {
    #if NCH_GLSDL_OPENGL_BACKEND>=1
    int rtW = getRenderTargetW();
    int rtH = getRenderTargetH();
    SDL_Rect rrect;
    if(!rect) {
        rrect = {0, 0, rtW, rtH};
    } else {
        rrect = { rect->x, rect->y, rect->w, rect->h };
    }

    //Create pixel array 'tempPixels' copied from texture
    unsigned char* tempPixels = new unsigned char[rtW*rtH*4];  // RGBA format
    if(renderTarget) {
        glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLSDL_Texture*>(renderTarget)->getGL_TextureID()); {
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempPixels);
        } glBindTexture(GL_TEXTURE_2D, 0);        
    } else {
        glReadPixels(0, 0, rtW, rtH, GL_RGBA, GL_UNSIGNED_BYTE, tempPixels);
        //Must flip pixels upside down if capturing window
        for(int y = 0; y<rtH/2; y++) {
        for(int x = 0; x<rtW; x++) {
            int topIdx = (y*rtW+x)*4;
            int bottomIdx = ((rtH-1-y)*rtW+x)*4;
            //Swap the RGBA values of the top and bottom rows
            for(int c = 0; c<4; c++) {
                std::swap(tempPixels[topIdx+c], tempPixels[bottomIdx+c]);
            }
        }}
    }


    int ret = 0;
    SDL_Surface* retSurf;
    SDL_PixelFormatEnum tempPixelsFmt = SDL_PIXELFORMAT_RGBA32;
    for(;;) {
        //Build 'tempSurf' which is an ABGR surface copy of the entire render target.
        SDL_Surface* tempSurf = SDL_CreateRGBSurfaceWithFormatFrom(tempPixels, rtW, rtH, 32, rtW*4, tempPixelsFmt);
        if(!tempSurf) {
            delete[] tempPixels;
            ret = -1; break;
        }
        //Build 'subSurf' which is a surface fitting the desired 'rect'.
        SDL_Surface* subSurf = SDL_CreateRGBSurfaceWithFormat(0, rrect.w, rrect.h, 32, tempPixelsFmt);
        if(!subSurf) {
            SDL_FreeSurface(tempSurf);
            delete[] tempPixels;
            ret = -2; break;
        }
        //Copy the appropriate area from 'tempSurf' to 'subSurf'. Discard 'tempSurf' and 'tempPixels'.
        if(SDL_BlitSurface(tempSurf, &rrect, subSurf, NULL)!=0) {
            SDL_FreeSurface(tempSurf);
            delete[] tempPixels;
            ret = -3; break;
        }
        SDL_FreeSurface(tempSurf);
        delete[] tempPixels;
        //Finally, convert 'subSurf' to the needed format & store into 'retSurf'. Discard 'subSurf'.
        retSurf = SDL_ConvertSurfaceFormat(subSurf, format, 0);
        SDL_FreeSurface(subSurf);
        if(!retSurf) { ret = -4; break; }
        //Ensure the provided pitch is correct
        if(pitch!=retSurf->w*retSurf->format->BytesPerPixel && pitch!=retSurf->pitch) {
            ret = -5; break;
        }
        break;
    }

    if(ret==0) {
        uint8_t* retPixels = (uint8_t*)retSurf->pixels;
        for(int y = 0; y<retSurf->h; y++) {
            // Copy one row of pixels from the source surface to the destination buffer
            memcpy((uint8_t*)pixels+y*pitch, retPixels+y*pitch, pitch);
        }
    } else {
        switch(ret) {
            case -1: { Log::error(__PRETTY_FUNCTION__, "Failed to create temp ABGR surface"); } break;
            case -2: { Log::error(__PRETTY_FUNCTION__, "Failed to create ABGR subsurface"); } break;
            case -3: { Log::error(__PRETTY_FUNCTION__, "Failed to blit temp surface to subsurface"); } break;
            case -4: { Log::error(__PRETTY_FUNCTION__, "Failed to convert subsurface to final surface"); } break;
            case -5: { Log::error(__PRETTY_FUNCTION__, "Incorrect pitch detected, does not match with width*bytesPerPixel"); }
        }
    }

    SDL_FreeSurface(retSurf);
    return ret;
#else
    return SDL_RenderReadPixels(sdlRenderer, rect, format, pixels, pitch);
#endif
}
int GLSDL_Renderer::setRenderDrawColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    renderDrawColor = { r/255.0f, g/255.0f, b/255.0f, a/255.0f };
    return 0;
#else
    return SDL_SetRenderDrawColor(sdlRenderer, r, g, b, a);
#endif
}
int GLSDL_Renderer::setRenderDrawBlendMode(SDL_BlendMode sdlBlendMode) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    renderDrawBlendMode = sdlBlendMode;
    return 0;
#else
    return SDL_SetRenderDrawBlendMode(sdlRenderer, sdlBlendMode);
#endif
}
int GLSDL_Renderer::setRenderTarget(void* texture) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(texture) {
        GLSDL_Texture* glsdlTexture = reinterpret_cast<GLSDL_Texture*>(texture);
        if(glsdlTexture->getSDL_AccessType()!=SDL_TEXTUREACCESS_TARGET) {
            return -1;
        }
    }

    renderTarget = texture;
    //Setup viewport and framebuffer
    vpUpdate();
    if(renderTarget!=nullptr) {
        glBindFramebuffer(GL_FRAMEBUFFER, reinterpret_cast<GLSDL_Texture*>(renderTarget)->getGL_FramebufferID());
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    return 0;
#else
    if(texture==nullptr) {
        return SDL_SetRenderTarget(sdlRenderer, nullptr);
    }
    return SDL_SetRenderTarget(sdlRenderer, reinterpret_cast<GLSDL_Texture*>(texture)->toSDL_Texture());
#endif
}
int GLSDL_Renderer::setRenderTargetClipRect(const SDL_Rect* rect) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    SDL_Rect cRect;
    bool cEnabled;

    if(rect==nullptr) {
        cRect = {0, 0, getRenderTargetW(), getRenderTargetH()};
        cEnabled = false;
    } else {
        if(rect->w<0 || rect->y<0) {
            return -1;
        }
        cRect = *rect;
        cEnabled = true;
    }

    GLSDL_Texture* glsdlTex = reinterpret_cast<GLSDL_Texture*>(renderTarget);
    if(glsdlTex==nullptr) {
        ntgtRenderClipEnabled = cEnabled;
        ntgtRenderClipRect = cRect;
        ntgtEffectiveRenderRect = calcEffectiveRenderRect(
            getRenderTargetW(), getRenderTargetH(), ntgtRenderViewport, ntgtRenderViewportEnabled, ntgtRenderClipRect, ntgtRenderClipEnabled
        );
    } else {
        glsdlTex->setRenderClipEnabled(cEnabled);
        glsdlTex->setRenderClipRect(cRect);
        glsdlTex->setEffectiveRenderRect(calcEffectiveRenderRect(
            getRenderTargetW(), getRenderTargetH(),
            glsdlTex->getRenderViewport(), glsdlTex->isRenderViewportEnabled(),
            glsdlTex->getRenderClipRect(), glsdlTex->isRenderClipEnabled()
        ));
    }
    

    return 0;
#else
    return SDL_RenderSetClipRect(sdlRenderer, rect);    
#endif
}
int GLSDL_Renderer::setRenderTargetViewport(const SDL_Rect* rect) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    SDL_Rect vpRect;
    bool vpEnabled;

    if(rect==nullptr) {
        vpRect = {0, 0, getRenderTargetW(), getRenderTargetH()};
        vpEnabled = false;
    } else {
        if(rect->w<0 || rect->y<0) {
            return -1;
        }
        vpRect = *rect;
        vpEnabled = true;
    }

    GLSDL_Texture* glsdlTex = reinterpret_cast<GLSDL_Texture*>(renderTarget);
    if(glsdlTex==nullptr) {
        ntgtRenderViewport = vpRect;
        ntgtRenderViewportEnabled = vpEnabled;
        ntgtEffectiveRenderRect = calcEffectiveRenderRect(
            getRenderTargetW(), getRenderTargetH(), ntgtRenderViewport, ntgtRenderViewportEnabled, ntgtRenderClipRect, ntgtRenderClipEnabled
        );
    } else {
        glsdlTex->setRenderViewport(vpRect);
        glsdlTex->setRenderViewportEnabled(vpEnabled);
        glsdlTex->setEffectiveRenderRect(calcEffectiveRenderRect(
            getRenderTargetW(), getRenderTargetH(),
            glsdlTex->getRenderViewport(), glsdlTex->isRenderViewportEnabled(),
            glsdlTex->getRenderClipRect(), glsdlTex->isRenderClipEnabled()
        ));
    }
    return 0;
#else
    return SDL_RenderSetViewport(sdlRenderer, rect);
#endif
}
int GLSDL_Renderer::renderFillRectF(SDL_FRect* rect)
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    SDL_FRect dst;
    if(rect==nullptr) {
        if(renderTarget!=nullptr && isRenderTargetViewportEnabled()) {
            const SDL_Rect& rtvp = getRenderTargetViewport();
            dst = {0, 0, (float)rtvp.w, (float)rtvp.h};
        } else {
            dst = {0, 0, (float)getRenderTargetW(), (float)getRenderTargetH()};
        }
    } else {
        dst = { rect->x, rect->y, rect->w, rect->h };
        dst.y = (float)getRenderTargetH()-dst.y-dst.h;
    }

    // Define the rectangle's vertices based on the given parameters
    const glm::vec4& rdr = renderDrawColor;
    std::vector<GLfloat> verts = {
        dst.x,       dst.y,       rdr.r, rdr.g, rdr.b, rdr.a,
        dst.x+dst.w, dst.y,       rdr.r, rdr.g, rdr.b, rdr.a,
        dst.x+dst.w, dst.y+dst.h, rdr.r, rdr.g, rdr.b, rdr.a,
        dst.x,       dst.y+dst.h, rdr.r, rdr.g, rdr.b, rdr.a
    };
    std::vector<GLuint> indices = { 0, 1, 2, 2, 3, 0 };
    translateVerts(renderTarget, getRenderTargetViewport(), verts.data(), 4, 6);

    //Generate buffers
    GLuint glVAO, glVBO, glEBO;
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);
    bool draw = false; {
        //Preparations for GL draw
        buildGL_SolidRenderBuffers(glVAO, glVBO, glEBO, verts, 6, indices);
        setupRenderTargetSettings(renderTarget, false);
        //Draw
        if(setGL_BlendFuncState(renderDrawBlendMode)) {
            setGL_ScissorState(this);
            glBindVertexArray(glVAO); {
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                draw = true;
            } glBindVertexArray(0);
        }
        //Cleanup after GL draw
        cleanupRenderTargetSettings(renderTarget);
    }
    //Cleanup buffers and return
    glDeleteVertexArrays(1, &glVAO);
    glDeleteBuffers(1, &glVBO);
    glDeleteBuffers(1, &glEBO);
    if(draw) return 0; return -1;
#else
    if(rect==nullptr) { return SDL_RenderFillRectF(sdlRenderer, nullptr); }
    else {
        SDL_FRect sdlRect = { rect->x, rect->y, rect->w, rect->h };
        return SDL_RenderFillRectF(sdlRenderer, &sdlRect);
    }
#endif
}
int GLSDL_Renderer::renderFillRect(SDL_Rect* rect) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(rect==nullptr) { return renderFillRectF(nullptr); } else {
        SDL_FRect dst = { (float)rect->x, (float)rect->y, (float)rect->w, (float)rect->h };
        return renderFillRectF(&dst);
    }
#else
    return SDL_RenderFillRect(sdlRenderer, rect);
#endif
}
int GLSDL_Renderer::renderDrawRectF(SDL_FRect* rect)
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    SDL_FRect dst;
    if(rect==nullptr) {
        if(renderTarget!=nullptr && isRenderTargetViewportEnabled()) {
            const SDL_Rect& rtvp = getRenderTargetViewport();
            dst = {0, 0, (float)rtvp.w, (float)rtvp.h};
        } else {
            dst = {0, 0, (float)getRenderTargetW(), (float)getRenderTargetH()};
        }
    } else {
        dst = { rect->x, rect->y, rect->w, rect->h };
        dst.y = (float)getRenderTargetH()-dst.y-dst.h;
    }

    //Define vertices and indices
    const glm::vec4& rdr = renderDrawColor;
    std::vector<GLfloat> verts = {
        dst.x+1,     dst.y,         rdr.r, rdr.g, rdr.b, rdr.a,
        dst.x+dst.w, dst.y,         rdr.r, rdr.g, rdr.b, rdr.a,
        dst.x+dst.w, dst.y+dst.h-1, rdr.r, rdr.g, rdr.b, rdr.a,
        dst.x+1,     dst.y+dst.h,   rdr.r, rdr.g, rdr.b, rdr.a,
    };
    std::vector<GLuint> indices = { 0, 1, 1, 2, 2, 3, 3, 0 };
    translateVerts(renderTarget, getRenderTargetViewport(), verts.data(), 4, 6);

    //Generate buffers
    GLuint glVAO, glVBO, glEBO;
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);
    bool draw = false; {
        //Preparations for GL draw
        buildGL_SolidRenderBuffers(glVAO, glVBO, glEBO, verts, 6, indices);
        setupRenderTargetSettings(renderTarget, false);
        //Draw
        if(setGL_BlendFuncState(renderDrawBlendMode)) {
            setGL_ScissorState(this);
            glBindVertexArray(glVAO); {
                glDrawElements(GL_LINES, sizeof(indices), GL_UNSIGNED_INT, 0);
                draw = true;
            } glBindVertexArray(0);
        }
        //Cleanup after GL draw
        cleanupRenderTargetSettings(renderTarget);
    }
    //Cleanup buffers and return
    glDeleteVertexArrays(1, &glVAO);
    glDeleteBuffers(1, &glVBO);
    glDeleteBuffers(1, &glEBO);
    if(draw) return 0; return -1;
#else
    return SDL_RenderDrawRectF(sdlRenderer, rect);
#endif
}
int GLSDL_Renderer::renderDrawRect(SDL_Rect* rect)
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(rect==nullptr) { return renderDrawRectF(nullptr); } else {
        SDL_FRect dst = { (float)rect->x, (float)rect->y, (float)rect->w, (float)rect->h };
        return renderDrawRectF(&dst);
    }
#else
    return SDL_RenderDrawRect(sdlRenderer, rect);
#endif
}
int GLSDL_Renderer::renderDrawLineF(float x1, float y1, float x2, float y2) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(!renderTarget) {
        y1 = (float)getRenderTargetH()-y1;
        y2 = (float)getRenderTargetH()-y2;
    }

    //Define the rectangle's vertices based on the given parameters
    const glm::vec4& rdr = renderDrawColor;
    std::vector<GLfloat> verts = {
        x1, y1, rdr.r, rdr.g, rdr.b, rdr.a,
        x2, y2, rdr.r, rdr.g, rdr.b, rdr.a,
    };
    std::vector<GLuint> indices = { 0, 1 };
    translateVerts(renderTarget, getRenderTargetViewport(), verts.data(), 2, 6);

    //Generate buffers
    GLuint glVAO, glVBO, glEBO;
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);
    bool draw = false; {
        //Preparations for GL draw
        buildGL_SolidRenderBuffers(glVAO, glVBO, glEBO, verts, 6, indices);
        setupRenderTargetSettings(renderTarget, false);
        //Draw
        if(setGL_BlendFuncState(renderDrawBlendMode)) {
            setGL_ScissorState(this);
            glBindVertexArray(glVAO); {
                glDrawElements(GL_LINES, sizeof(indices), GL_UNSIGNED_INT, 0);
                draw = true;
            } glBindVertexArray(0);
        }
        //Cleanup after GL draw
        cleanupRenderTargetSettings(renderTarget);
    }
    //Cleanup buffers and return
    glDeleteVertexArrays(1, &glVAO);
    glDeleteBuffers(1, &glVBO);
    glDeleteBuffers(1, &glEBO);
    if(draw) return 0; return -1;
#else
    return SDL_RenderDrawLineF(sdlRenderer, x1, y1, x2, y2);
#endif
}
int GLSDL_Renderer::renderDrawLine(int x1, int y1, int x2, int y2) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return renderDrawLineF(x1, y1, x2, y2);
#else
    return SDL_RenderDrawLine(sdlRenderer, x1, y1, x2, y2);    
#endif
}
int GLSDL_Renderer::renderDrawPointF(float x, float y) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(!renderTarget) {
        y = (float)getRenderTargetH()-y;
    }

    //Define the rectangle's vertices based on the given parameters
    const glm::vec4& rdr = renderDrawColor;
    std::vector<GLfloat> verts = {
        x, y, rdr.r, rdr.g, rdr.b, rdr.a,
    };
    std::vector<GLuint> indices = { 0 };
    translateVerts(renderTarget, getRenderTargetViewport(), verts.data(), 1, 6);

    //Generate buffers
    GLuint glVAO, glVBO, glEBO;
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);
    bool draw = false; {
        //Preparations for GL draw
        buildGL_SolidRenderBuffers(glVAO, glVBO, glEBO, verts, 6, indices);
        setupRenderTargetSettings(renderTarget, false);
        //Draw
        if(setGL_BlendFuncState(renderDrawBlendMode)) {
            setGL_ScissorState(this);
            glBindVertexArray(glVAO); {
                glDrawElements(GL_POINTS, sizeof(indices), GL_UNSIGNED_INT, 0);
                draw = true;
            } glBindVertexArray(0);
        }
        //Cleanup after GL draw
        cleanupRenderTargetSettings(renderTarget);
    }
    //Cleanup buffers and return
    glDeleteVertexArrays(1, &glVAO);
    glDeleteBuffers(1, &glVBO);
    glDeleteBuffers(1, &glEBO);
    if(draw) return 0; return -1;
#else
    return SDL_RenderDrawPointF(sdlRenderer, x, y);    
#endif
}
int GLSDL_Renderer::renderDrawPoint(int x, int y) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return renderDrawPointF(x, y);
#else
    return SDL_RenderDrawPoint(sdlRenderer, x, y);    
#endif
}
int GLSDL_Renderer::renderCopyExF(void* texture, SDL_Rect* srcrect, SDL_FRect* dstrect, double angle, SDL_FPoint* center, SDL_RendererFlip flip) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    GLSDL_Texture* glsdlTexture = reinterpret_cast<GLSDL_Texture*>(texture);

    //Find dst
    SDL_FRect dst;
    if(dstrect==nullptr) {
        if(renderTarget!=nullptr && isRenderTargetViewportEnabled()) {
            const SDL_Rect& rtvp = getRenderTargetViewport();
            dst = {0, 0, (float)rtvp.w, (float)rtvp.h};
        } else {
            dst = {0, 0, (float)getRenderTargetW(), (float)getRenderTargetH()};
        }
    } else {
        dst = { (float)dstrect->x, (float)dstrect->y, (float)dstrect->w, (float)dstrect->h };
        dst.y = getRenderTargetH()-dst.y-dst.h;
    }
    //Find src
    SDL_FRect src;
    if(srcrect==nullptr) {
        src = {0, 0, 1, 1};
    } else {
        float texW = glsdlTexture->getGL_TexWidth();
        float texH = glsdlTexture->getGL_TexHeight();
        src = {srcrect->x/texW, srcrect->y/texH, srcrect->w/texW, srcrect->h/texH};
    }
    if(renderTarget!=nullptr) {
        src.y += src.h;
        src.h *= -1.f;
    }

    //Define the rectangle's vertices based on the given parameters.
    const glm::vec4& tcm = glsdlTexture->getSDL_TexColorMod();
    std::vector<GLfloat> verts = {
        dst.x,       dst.y,       tcm.r, tcm.g, tcm.b, tcm.a, src.x,       src.y+src.h,
        dst.x+dst.w, dst.y,       tcm.r, tcm.g, tcm.b, tcm.a, src.x+src.w, src.y+src.h,
        dst.x+dst.w, dst.y+dst.h, tcm.r, tcm.g, tcm.b, tcm.a, src.x+src.w, src.y,
        dst.x,       dst.y+dst.h, tcm.r, tcm.g, tcm.b, tcm.a, src.x,       src.y
    };
    std::vector<GLuint> indices = { 0, 1, 2, 2, 3, 0 };
    if(angle!=0 || !center || flip!=SDL_FLIP_NONE) {
        transformQuadVertsF(getRenderTargetH(), verts.data(), 8, angle, center, flip);
    }
    translateVerts(renderTarget, getRenderTargetViewport(), verts.data(), 4, 8);

    //Generate buffers
    GLuint glVAO, glVBO, glEBO;
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);
    bool draw = false; {
        //Preparations for GL draw
        buildGL_TexedRenderBuffers(glVAO, glVBO, glEBO, verts, 8, indices);
        setupRenderTargetSettings(renderTarget, true);
        //Draw (textured)
        if(texture && setGL_BlendFuncState(glsdlTexture->getSDL_BlendMode())) {
            setGL_ScissorState(this);
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, glsdlTexture->getGL_TextureID()); {
                glBindVertexArray(glVAO); {
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    draw = true;
                } glBindVertexArray(0);
            } glBindTexture(GL_TEXTURE_2D, 0);            
        }

        //Cleanup after GL draw
        cleanupRenderTargetSettings(renderTarget);
    }
    //Cleanup buffers and return
    glDeleteVertexArrays(1, &glVAO);
    glDeleteBuffers(1, &glVBO);
    glDeleteBuffers(1, &glEBO);
    if(draw) return 0; return -1;
#else
    SDL_Rect src; SDL_Rect* s = nullptr;
    if(srcrect!=nullptr) {
        src = { srcrect->x, srcrect->y, srcrect->w, srcrect->h };
        s = &src;
    }
    SDL_FRect dst; SDL_FRect* d = nullptr;
    if(dstrect!=nullptr) {
        dst = { dstrect->x, dstrect->y, dstrect->w, dstrect->h };
        d = &dst;
    }
    return SDL_RenderCopyExF(sdlRenderer, reinterpret_cast<GLSDL_Texture*>(texture)->toSDL_Texture(), s, d, angle, center, flip);
#endif
}
int GLSDL_Renderer::renderCopyEx(void* texture, SDL_Rect* srcrect, SDL_Rect* dstrect, double angle, SDL_Point* center, SDL_RendererFlip flip) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    GLSDL_Texture* glsdlTexture = reinterpret_cast<GLSDL_Texture*>(texture);

    //Find dst
    SDL_FRect dst;
    if(dstrect==nullptr) {
        if(renderTarget!=nullptr && isRenderTargetViewportEnabled()) {
            const SDL_Rect& rtvp = getRenderTargetViewport();
            dst = {0, 0, (float)rtvp.w, (float)rtvp.h};
        } else {
            dst = {0, 0, (float)getRenderTargetW(), (float)getRenderTargetH()};
        }
    } else {
        dst = { (float)dstrect->x, (float)dstrect->y, (float)dstrect->w, (float)dstrect->h };
        dst.y = getRenderTargetH()-dst.y-dst.h;
    }
    //Find src
    SDL_FRect src;
    if(srcrect==nullptr) {
        src = {0, 0, 1, 1};
    } else {
        float texW = glsdlTexture->getGL_TexWidth();
        float texH = glsdlTexture->getGL_TexHeight();
        src = {srcrect->x/texW, srcrect->y/texH, srcrect->w/texW, srcrect->h/texH};
    }
    if(renderTarget!=nullptr) {
        src.y += src.h;
        src.h *= -1.f;
    }

    //Define the rectangle's vertices based on the given parameters.
    const glm::vec4& tcm = glsdlTexture->getSDL_TexColorMod();
    std::vector<GLfloat> verts = {
        dst.x,       dst.y,       tcm.r, tcm.g, tcm.b, tcm.a, src.x,       src.y+src.h,
        dst.x+dst.w, dst.y,       tcm.r, tcm.g, tcm.b, tcm.a, src.x+src.w, src.y+src.h,
        dst.x+dst.w, dst.y+dst.h, tcm.r, tcm.g, tcm.b, tcm.a, src.x+src.w, src.y,
        dst.x,       dst.y+dst.h, tcm.r, tcm.g, tcm.b, tcm.a, src.x,       src.y
    };
    std::vector<GLuint> indices = { 0, 1, 2, 2, 3, 0 };
    if(angle!=0 || !center || flip!=SDL_FLIP_NONE) {
        transformQuadVertsI(getRenderTargetH(), verts.data(), 8, angle, center, flip);
    }
    translateVerts(renderTarget, getRenderTargetViewport(), verts.data(), 4, 8);

    //Generate buffers
    GLuint glVAO, glVBO, glEBO;
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);
    bool draw = false; {
        //Preparations for GL draw
        buildGL_TexedRenderBuffers(glVAO, glVBO, glEBO, verts, 8, indices);
        setupRenderTargetSettings(renderTarget, true);
        //Draw (textured)
        if(texture && setGL_BlendFuncState(glsdlTexture->getSDL_BlendMode())) {
            setGL_ScissorState(this);
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, glsdlTexture->getGL_TextureID()); {
                glBindVertexArray(glVAO); {
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    draw = true;
                } glBindVertexArray(0);
            } glBindTexture(GL_TEXTURE_2D, 0);            
        }

        //Cleanup after GL draw
        cleanupRenderTargetSettings(renderTarget);
    }
    //Cleanup buffers and return
    glDeleteVertexArrays(1, &glVAO);
    glDeleteBuffers(1, &glVBO);
    glDeleteBuffers(1, &glEBO);
    if(draw) return 0; return -1;
#else
    return SDL_RenderCopyEx(sdlRenderer, reinterpret_cast<GLSDL_Texture*>(texture)->toSDL_Texture(), srcrect, dstrect, angle, center, flip);
#endif
}
int GLSDL_Renderer::renderCopyF(void* texture, SDL_Rect* srcrect, SDL_FRect* dstrect) {
    return renderCopyExF(texture, srcrect, dstrect, 0, nullptr, SDL_FLIP_NONE);
}
int GLSDL_Renderer::renderCopy(void* texture, SDL_Rect* srcrect, SDL_Rect* dstrect) {
    return renderCopyEx(texture, srcrect, dstrect, 0, nullptr, SDL_FLIP_NONE);
}
int GLSDL_Renderer::renderGeometry(void* texture, const SDL_Vertex* vertices, int numVerts, const int* indices, int numInds) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    GLSDL_Texture* glsdlTexture = reinterpret_cast<GLSDL_Texture*>(texture);

    constexpr int vStride = 8;
    std::vector<GLfloat> glVerts; glVerts.resize(vStride*numVerts);
    for(int i = 0; i<numVerts; i++) {
        glVerts[vStride*i+0] = vertices[i].position.x;
        if(renderTarget!=nullptr) {
            glVerts[vStride*i+1] = vertices[i].position.y;
        } else {
            glVerts[vStride*i+1] = getRenderTargetH()-vertices[i].position.y;
        }
        
        glVerts[vStride*i+2] = vertices[i].color.r/255.0f;
        glVerts[vStride*i+3] = vertices[i].color.g/255.0f;
        glVerts[vStride*i+4] = vertices[i].color.b/255.0f;
        glVerts[vStride*i+5] = vertices[i].color.a/255.0f;
        glVerts[vStride*i+6] = vertices[i].tex_coord.x;
        glVerts[vStride*i+7] = vertices[i].tex_coord.y;
    }
    std::vector<GLuint> glInds; glInds.resize(numInds);
    for(int i = 0; i<numInds; i++) {
        glInds[i] = indices[i];
    }
    translateVerts(renderTarget, getRenderTargetViewport(), glVerts.data(), numVerts, vStride);

    //Generate buffers
    GLuint glVAO, glVBO, glEBO;
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);
    bool draw = false; {
        //Preparations for GL draw
        if(glsdlTexture==nullptr) { buildGL_SolidRenderBuffers(glVAO, glVBO, glEBO, glVerts, vStride, glInds); }
        else                      { buildGL_TexedRenderBuffers(glVAO, glVBO, glEBO, glVerts, vStride, glInds); }
        setupRenderTargetSettings(renderTarget, glsdlTexture!=nullptr);
        //Draw textured or solid
        {
            //Set blending only if textured
            if(glsdlTexture) {
                if(!setGL_BlendFuncState(glsdlTexture->getSDL_BlendMode())) {
                    draw = false;
                }
            }
            //Set scissor
            setGL_ScissorState(this);
            //Bind to tex only if textured
            if(glsdlTexture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, glsdlTexture->getGL_TextureID());
            }
            //Draw
            glBindVertexArray(glVAO); {
                glDrawElements(GL_TRIANGLES, glInds.size(), GL_UNSIGNED_INT, 0);
                draw = true;
            } glBindVertexArray(0);
            //Unbind from tex only if textured
            if(glsdlTexture) {
                glBindTexture(GL_TEXTURE_2D, 0);            
            }
        }

        //Cleanup after GL draw
        cleanupRenderTargetSettings(renderTarget);
    }
    //Cleanup buffers and return
    glDeleteVertexArrays(1, &glVAO);
    glDeleteBuffers(1, &glVBO);
    glDeleteBuffers(1, &glEBO);
    if(draw) return 0; return -1;
#else
    if(texture==nullptr) {
        return SDL_RenderGeometry(sdlRenderer, nullptr, vertices, numVerts, indices, numInds);
    }
    return SDL_RenderGeometry(sdlRenderer, reinterpret_cast<GLSDL_Texture*>(texture)->toSDL_Texture(), vertices, numVerts, indices, numInds);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLSDL_Texture::GLSDL_Texture(GLSDL_Renderer* glsdlRenderer, SDL_Surface* surface)
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    //Create GL texture from surface properties
    createGL_Texture(surface->format->format, SDL_TEXTUREACCESS_STATIC, surface->w, surface->h, true);

	/* Populate GL texture by converting SDL surface to the proper pixel format */
    glBindTexture(GL_TEXTURE_2D, glTexture); {
        //Create final surface (which may be converted) whose pixels will be used
		SDL_Surface* finalSurf = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
        if(finalSurf==NULL) {
            Log::errorv(__PRETTY_FUNCTION__, "IMG Error", IMG_GetError());
            failedConstruction = true; return;
        }
        //Populate GL texture wth final surface's pixels
        glTexImage2D(GL_TEXTURE_2D, 0, glTexFormat, finalSurf->w, finalSurf->h, 0, GL_RGBA, glTexDataType, finalSurf->pixels);
		//Cleanup final surface
        SDL_FreeSurface(finalSurf);
	} glBindTexture(GL_TEXTURE_2D, 0);
#else
    sdlRenderer = glsdlRenderer->toSDL_Renderer();
    if(sdlRenderer==nullptr) {
        failedConstruction = true; return;
    }    
    sdlTexture = SDL_CreateTextureFromSurface(glsdlRenderer->toSDL_Renderer(), surface);
    if(sdlTexture==nullptr) {
        failedConstruction = true; return;
    }
#endif

}
GLSDL_Texture::GLSDL_Texture(GLSDL_Renderer* glsdlRenderer, uint32_t format, int access, int w, int h)
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    createGL_Texture(format, access, w, h);
#else
    sdlRenderer = glsdlRenderer->toSDL_Renderer();
    if(sdlRenderer==nullptr) {
        failedConstruction = true; return;
    }
    sdlTexture = SDL_CreateTexture(sdlRenderer, format, access, w, h);
    if(sdlTexture==nullptr) {
        failedConstruction = true; return;
    }
#endif
}
GLSDL_Texture::~GLSDL_Texture() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(glTexture!=0) {
        glDeleteTextures(1, &glTexture);
        glTexture = 0;
    }
    if(glFBO!=0) {
        glDeleteFramebuffers(1, &glFBO);
        glFBO = 0;
    }
    if(glPBO!=0) {
        glDeleteBuffers(1, &glPBO);
        glPBO = 0;
    }
#else
    SDL_DestroyTexture(sdlTexture);
#endif

}

bool GLSDL_Texture::hasFailedConstruction() const {
    return failedConstruction;
}
SDL_Texture* GLSDL_Texture::toSDL_Texture() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    throw std::invalid_argument("Function only usable in raw SDL");
#else
    return sdlTexture;
#endif
}
GLuint GLSDL_Texture::getGL_TextureID() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return glTexture;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
GLuint GLSDL_Texture::getGL_FramebufferID() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return glFBO;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
int GLSDL_Texture::getGL_TexWidth() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return glTexW;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
int GLSDL_Texture::getGL_TexHeight() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return glTexH;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
int GLSDL_Texture::getSDL_AccessType() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return sdlAccessType;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
glm::vec4 GLSDL_Texture::getSDL_TexColorMod() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return sdlTexColorMod;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
SDL_Color GLSDL_Texture::getSDL_TexColorModRaw() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return sdlTexColorModRaw;
#else
    SDL_Color ret;
    SDL_GetTextureColorMod(sdlTexture, &ret.r, &ret.g, &ret.b);
    SDL_GetTextureAlphaMod(sdlTexture, &ret.a);
    return ret;
#endif
}
SDL_BlendMode GLSDL_Texture::getSDL_BlendMode() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return sdlBlendMode;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
uint32_t GLSDL_Texture::getSDL_PixelFormat() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return sdlPixelFormat;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}

int GLSDL_Texture::queryTextureAlphaMod(uint8_t* a) const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(a==NULL) return -1;
    *a = sdlTexColorModRaw.a;
    return 0;
#else
    return SDL_GetTextureAlphaMod(sdlTexture, a);
#endif
}
int GLSDL_Texture::queryTextureColorMod(uint8_t* r, uint8_t* g, uint8_t* b) const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(r==NULL) return -1;
    if(g==NULL) return -1;
    if(b==NULL) return -1;
    *r = sdlTexColorModRaw.r;
    *g = sdlTexColorModRaw.g;
    *b = sdlTexColorModRaw.b;
    return 0;
#else
    return SDL_GetTextureColorMod(sdlTexture, r, g, b);
#endif
}
int GLSDL_Texture::queryTextureBlendMode(SDL_BlendMode* sdlBlendMode) const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(sdlBlendMode==NULL) return -1;
    *sdlBlendMode = getSDL_BlendMode();
    return 0;
#else
    return SDL_GetTextureBlendMode(sdlTexture, sdlBlendMode);
#endif
}
int GLSDL_Texture::query(uint32_t* format, int* access, int* w, int* h) const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(format!=NULL) *format = sdlPixelFormat;
    if(access!=NULL) *access = sdlAccessType;
    if(w!=NULL)      *w = glTexW;
    if(h!=NULL)      *h = glTexH;
    return 0;
#else
    return SDL_QueryTexture(sdlTexture, format, access, w, h);
#endif

}
SDL_Rect GLSDL_Texture::getRenderClipRect() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return renderClipRect;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
bool GLSDL_Texture::isRenderClipEnabled() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return renderClipEnabled;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
SDL_Rect GLSDL_Texture::getRenderViewport() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return renderViewport;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
bool GLSDL_Texture::isRenderViewportEnabled() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return renderViewportEnabled;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
SDL_Rect GLSDL_Texture::getEffectiveRenderRect() const {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    return effectiveRenderArea;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}

int GLSDL_Texture::update(const SDL_Rect* rect, const void* pixels, int pitch) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(sdlAccessType!=SDL_TEXTUREACCESS_STREAMING) {
        Log::error(__PRETTY_FUNCTION__, "Only SDL_TEXTUREACCESS_STREAMING textures support lock/unlock/update");
        return -1;
    }
    if(locked) {
        Log::error(__PRETTY_FUNCTION__, "Texture has already been locked");
        return -1;
    }

    SDL_Rect tempLkRect;
    if(rect==nullptr) { tempLkRect = {0, 0, glTexW, glTexH}; }
    else              { tempLkRect = *rect; }

    //Lock the texture for direct access to its pixels
    void* tempLkPixels;
    int tempLkPitch;
    if(lock(&tempLkRect, &tempLkPixels, &tempLkPitch)!=0) {
        Log::error(__PRETTY_FUNCTION__, "Failed locking component during texture update");
        return -1;
    }
    //Iterate over the height and width of the 'tempLkRect' and copy the pixel data.
    uint8_t* lkPixelData = (uint8_t*)tempLkPixels;
    int xOffset = tempLkRect.x*glPBO_BPP;
    int yOffset = tempLkRect.y*tempLkPitch;
    for(int y = 0; y<tempLkRect.h; y++) {
        uint8_t *row = lkPixelData+yOffset + y*tempLkPitch + xOffset;
        //Copy the pixel data for the current row.
        memcpy(row, (uint8_t*)pixels+y*pitch, tempLkRect.w * 4);
    }
    //Unlock the texture after updating the pixels.
    unlock();
    return true;

    return 0;
#else
    return SDL_UpdateTexture(sdlTexture, rect, pixels, pitch);
#endif
}
int GLSDL_Texture::lock(const SDL_Rect* rect, void** pixels, int* pitch) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(sdlAccessType!=SDL_TEXTUREACCESS_STREAMING) {
        Log::error(__PRETTY_FUNCTION__, "Only SDL_TEXTUREACCESS_STREAMING textures support lock/unlock/update");
        return -1;
    }
    if(locked) {
        Log::error(__PRETTY_FUNCTION__, "Texture has already been locked");
        return -1;
    }
    if(pitch==nullptr) {
        Log::error(__PRETTY_FUNCTION__, "Pointer to 'pitch' is nullptr");
        return -1;
    }

    if(rect==nullptr) { lkRect = {0, 0, glTexW, glTexH}; }
    else              { lkRect = *rect; }

    if(lkPixels!=nullptr) free(lkPixels);
    lkPixels = calloc(lkRect.w*lkRect.h, glPBO_BPP);
    
    *pixels = lkPixels;
    lkPitch = lkRect.w*glPBO_BPP;
    *pitch = lkPitch;
    
    locked = true;
    return 0;
#else
    return SDL_LockTexture(sdlTexture, rect, pixels, pitch);
#endif
}
void GLSDL_Texture::unlock() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(sdlAccessType!=SDL_TEXTUREACCESS_STREAMING) {
        Log::error(__PRETTY_FUNCTION__, "Only SDL_TEXTUREACCESS_STREAMING textures support lock/unlock/update");
        return;
    }
    if(!locked) {
        Log::warnv(__PRETTY_FUNCTION__, "doing nothing", "Texture is not locked");
        return;
    }

    /* Update GL texture w/ 'lockedPixels' */
    {
        int ret = 0;
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glPBO); {
            GLubyte* ptr = (GLubyte*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, lkRect.h*lkPitch, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT); {
                if(ptr) {
                    //Copy the pixel data to the mapped memory
                    memcpy(ptr, lkPixels, lkRect.h*lkPitch);
                    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                } else {
                    ret = -1;
                }
            }
            if(ret==0) {
                //Now update the texture using glTexSubImage2D
                glBindTexture(GL_TEXTURE_2D, glTexture);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glPBO); // Make sure we're using the PBO
                glTexSubImage2D(GL_TEXTURE_2D, 0, lkRect.x, lkRect.y, lkRect.w, lkRect.h, glTexUpdateFormat, GL_UNSIGNED_BYTE, nullptr);
            }
        } glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    locked = false;
    lkRect = {0,0,0,0};
    if(lkPixels!=nullptr) free(lkPixels); lkPixels = nullptr;
    lkPitch = 0;
    return;
#else
    SDL_UnlockTexture(sdlTexture);
#endif
}
int GLSDL_Texture::setColorMod(uint8_t r, uint8_t g, uint8_t b) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    sdlTexColorMod.r = r/255.0f;
    sdlTexColorMod.g = g/255.0f;
    sdlTexColorMod.b = b/255.0f;
    sdlTexColorModRaw.r = r;
    sdlTexColorModRaw.g = g;
    sdlTexColorModRaw.b = b;
    return 0;
#else
    return SDL_SetTextureColorMod(sdlTexture, r, g, b);
#endif
}
int GLSDL_Texture::setAlphaMod(uint8_t a) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    sdlTexColorMod.a = a/255.0f;
    sdlTexColorModRaw.a = a;
    return 0;
#else
    return SDL_SetTextureAlphaMod(sdlTexture, a);
#endif
}
int GLSDL_Texture::setBlendMode(SDL_BlendMode sdlBlendMode) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    GLSDL_Texture::sdlBlendMode = sdlBlendMode;
    return 0;
#else
    return SDL_SetTextureBlendMode(sdlTexture, sdlBlendMode);
#endif
}
void GLSDL_Texture::setRenderClipRect(const SDL_Rect& rect) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    renderClipRect = rect;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Texture::setRenderClipEnabled(bool enabled) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    renderClipEnabled = enabled;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Texture::setRenderViewport(const SDL_Rect& rect) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    renderViewport = rect;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Texture::setRenderViewportEnabled(bool enabled) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    renderViewportEnabled = enabled;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Texture::setEffectiveRenderRect(const SDL_Rect& rect) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    effectiveRenderArea = rect;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}

void GLSDL_Texture::createGL_Texture(uint32_t sdlPixelFormat, int sdlAccessType, int w, int h, bool texFromSurface)
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    //Width check
    if(w<=0 || h<=0) {
        failedConstruction = true;
        return;
    }
    //Access type check
    if(sdlAccessType!=SDL_TEXTUREACCESS_STATIC && sdlAccessType!=SDL_TEXTUREACCESS_STREAMING && sdlAccessType!=SDL_TEXTUREACCESS_TARGET) {
        failedConstruction = true;
        return;
    }
    //Convert SDL pixel format to closest one from OpenGL that works.
    //To be as safe as possible you should try to rely on the "common" SDL pixelformats like RGBA8888/32, or ABGR8888/32.
    glTexDataType = GL_UNSIGNED_BYTE;
    bool hasAlpha = false;
    switch(sdlPixelFormat) {
        case SDL_PIXELFORMAT_INDEX8:
        {
            hasAlpha = false;
            glTexFormat = GL_RED;
            glTexUpdateFormat = GL_RED;
            glPBO_BPP = 1;
        } break;
        case SDL_PIXELFORMAT_RGB332:
        {
            hasAlpha = false; 
            glTexFormat = GL_R3_G3_B2;
            glTexUpdateFormat = GL_R3_G3_B2;
            glPBO_BPP = 1;
        } break;
        case SDL_PIXELFORMAT_XRGB4444:
        case SDL_PIXELFORMAT_XBGR4444:
        case SDL_PIXELFORMAT_XRGB1555:
        case SDL_PIXELFORMAT_XBGR1555:
        {
            hasAlpha = false; 
            glTexFormat = GL_RGB4;
            glPBO_BPP = 2;
        } break;
        case SDL_PIXELFORMAT_ARGB4444:
        case SDL_PIXELFORMAT_RGBA4444:
        case SDL_PIXELFORMAT_BGRA4444:
        case SDL_PIXELFORMAT_ARGB1555:
        case SDL_PIXELFORMAT_RGBA5551:
        case SDL_PIXELFORMAT_ABGR1555:
        case SDL_PIXELFORMAT_BGRA5551: {
            hasAlpha = true;
            glTexFormat = GL_RGBA4;
            glPBO_BPP = 2;
        } break;
        case SDL_PIXELFORMAT_RGB565:
        case SDL_PIXELFORMAT_BGR565: {
            hasAlpha = false;
            glTexFormat = GL_RGB565;
            glPBO_BPP = 2;
        } break;
        case SDL_PIXELFORMAT_RGB24:
        case SDL_PIXELFORMAT_BGR24:
        case SDL_PIXELFORMAT_XRGB8888:
        case SDL_PIXELFORMAT_RGBX8888:
        case SDL_PIXELFORMAT_XBGR8888:
        case SDL_PIXELFORMAT_BGRX8888:
        {
            hasAlpha = false; 
            glTexFormat = GL_RGB8;
            glPBO_BPP = 3;
        } break;

        case SDL_PIXELFORMAT_ARGB8888:
        case SDL_PIXELFORMAT_RGBA8888:
        case SDL_PIXELFORMAT_ABGR8888:
        case SDL_PIXELFORMAT_BGRA8888:
        case SDL_PIXELFORMAT_ARGB2101010:
        {
            hasAlpha = true;
            glTexFormat = GL_RGBA8;
            glTexUpdateFormat = GL_ABGR_EXT;
            glPBO_BPP = 4;
        } break;
        default: {
            Log::errorv(__PRETTY_FUNCTION__, "sdlPixelFormat", "Unimplemented pixel format \"%s\" (%d)", SDL_GetPixelFormatName(sdlPixelFormat), sdlPixelFormat);
            failedConstruction = true; return;
        } break;
    }

    //Set members
    GLSDL_Texture::sdlPixelFormat = sdlPixelFormat;
    GLSDL_Texture::sdlAccessType = sdlAccessType;
    glTexW = w;
    glTexH = h;
    if(texFromSurface && hasAlpha) {
        sdlBlendMode = SDL_BLENDMODE_BLEND;
    } else {
        sdlBlendMode = SDL_BLENDMODE_NONE;
    }

    //Create
    glGenTextures(1, &glTexture);
    glBindTexture(GL_TEXTURE_2D, glTexture); {
        //Allocate storage only (no data uploaded)
        glTexImage2D(GL_TEXTURE_2D, 0, glTexFormat, w, h, 0, GL_RGBA, glTexDataType, nullptr);
        //Set filtering params
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //Set wrapping params
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } glBindTexture(GL_TEXTURE_2D, 0);

    switch(sdlAccessType) {
    case SDL_TEXTUREACCESS_TARGET: {
        //Create FBO (frame buffer object) if texture should be a render target
        glGenFramebuffers(1, &glFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, glFBO); {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTexture, 0);
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				Log::errorv(__PRETTY_FUNCTION__, "glCheckFramebufferStatus", "Incomplete framebuffer was created");
                failedConstruction = true;
            }
        } glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } break;
    case SDL_TEXTUREACCESS_STREAMING: {
        //Create PBO (pixel buffer object) if texture should be streamed to
        if(glTexUpdateFormat==0) {
            Log::errorv(__PRETTY_FUNCTION__, "sdlPixelFormat", "Only 4 bytes per pixel SDL_PixelFormats are supported for SDL_TEXTUREACCESS_STREAMING textures");
            failedConstruction = true; return;
        } else {
            glGenBuffers(1, &glPBO);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glPBO); {
                glBufferData(GL_PIXEL_UNPACK_BUFFER, glTexW*glTexH*glPBO_BPP, nullptr, GL_STREAM_DRAW);
            } glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
    } break;
    case SDL_TEXTUREACCESS_STATIC: {
        //No action needed.
    } break;
    }
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
#endif