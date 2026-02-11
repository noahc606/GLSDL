#pragma once
#ifdef NCH_GLSDL_OPENGL_BACKEND
#include <GL/glew.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>
#include "GLSDL_video.h"
#include "GLSDL_shader.h"

class GLSDL_Renderer {
public:
    GLSDL_Renderer(GLSDL_Window* window, int index, uint32_t flags);
    ~GLSDL_Renderer();

    bool hasFailedConstruction() const;
    SDL_Renderer* toSDL_Renderer() const;
    SDL_GLContext getSDL_GLContext() const;
    int getRenderDrawBlendMode(SDL_BlendMode* sdlBlendMode) const;
    int getRenderDrawColor(uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a) const;
    void* getRenderTarget() const;
    int getRenderTargetW() const;
    int getRenderTargetH() const;
    SDL_Rect getRenderTargetClipRect() const; bool isRenderTargetClipEnabled() const;
    SDL_Rect getRenderTargetViewport() const; bool isRenderTargetViewportEnabled() const;
    SDL_Rect getEffectiveRenderTargetRect() const;
    static void transformQuadVertsF(int rtH, float* verts, int stride, float angleDegrees, SDL_FPoint* center, SDL_RendererFlip flip);
    static void transformQuadVertsI(int rtH, float* verts, int stride, float angleDegrees, SDL_Point* center, SDL_RendererFlip flip);
    static void translateVerts(void* renderTarget, const SDL_Rect& viewport, float* verts, int numVerts, int stride);
    static SDL_Rect calcEffectiveRenderRect(int rtgtW, int rtgtH, const SDL_Rect& viewport, bool viewportEnabled, const SDL_Rect& clipRect, bool clipEnabled);

    static void glSaveState(GLSDL_Renderer* glsdlRenderer);
    static void assertGL_SaveStateExists();
    static void glRestoreState();
    int renderClear();
    void renderPresent();
    int renderReadPixels(const SDL_Rect* rect, uint32_t format, void* pixels, int pitch);
    int setRenderDrawColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    int setRenderDrawBlendMode(SDL_BlendMode blendMode);
    int setRenderTarget(void* texture);
    int setRenderTargetClipRect(const SDL_Rect* rect);
    int setRenderTargetViewport(const SDL_Rect* rect);

    int renderFillRectF(SDL_FRect* rect);
    int renderFillRect(SDL_Rect* rect);
    int renderDrawRectF(SDL_FRect* rect);
    int renderDrawRect(SDL_Rect* rect);
    int renderDrawLineF(float x1, float y1, float x2, float y2);
    int renderDrawLine(int x1, int y1, int x2, int y2);
    int renderDrawPointF(float x, float y);
    int renderDrawPoint(int x, int y);
    int renderCopyExF(void* texture, SDL_Rect* srcrect, SDL_FRect* dstrect, double angle, SDL_FPoint* center, SDL_RendererFlip flip);
    int renderCopyEx(void* texture, SDL_Rect* srcrect, SDL_Rect* dstrect, double angle, SDL_Point* center, SDL_RendererFlip flip);
    int renderCopyF(void* texture, SDL_Rect* srcrect, SDL_FRect* dstrect);
    int renderCopy(void* texture, SDL_Rect* srcrect, SDL_Rect* dstrect);
    int renderGeometry(void* texture, const SDL_Vertex* vertices, int numVerts, const int* indices, int numInds);

private:
    //Private GL state modification
    void buildGL_SolidRenderBuffers(GLuint glVAO, GLuint glVBO, GLuint glEBO, const std::vector<GLfloat>& verts, int vStride, const std::vector<GLuint>& indices);
    void buildGL_TexedRenderBuffers(GLuint glVAO, GLuint glVBO, GLuint glEBO, const std::vector<GLfloat>& verts, int vStride, const std::vector<GLuint>& indices);
    void setupRenderTargetSettings(void* renderTarget, bool applyTexture);
    void cleanupRenderTargetSettings(void* renderTarget);
    static void saveGL_ClearColor();
    static void restoreGL_ClearColor();
    static void saveGL_Viewport();
    static void restoreGL_Viewport();
    static void saveGL_BlendFuncState();
    static bool setGL_BlendFuncState(SDL_BlendMode sdlBlendMode);
    static void restoreGL_BlendFuncState();
    static void saveGL_ScissorState();
    static void setGL_ScissorState(GLSDL_Renderer* glsdlRenderer);
    static void restoreGL_ScissorState();
    static void saveGL_CullFaceState();
    static void restoreGL_CullFaceState();
    static void saveGL_DepthTestState();
    static void restoreGL_DepthTestState();
    static void saveGL_ActiveTextureState();
    static void restoreGL_ActiveTextureState();
    static void saveGL_UsedProgram();
    static void restoreGL_UsedProgram();

    void vpUpdate();
    void sdrUniform2DProjection(float w, float h);
    void sdrUniform2DProjection();
    void sdrUniformApplyTexture(bool applyTexture);

    //Original SDL2 behavior
#if NCH_GLSDL_OPENGL_BACKEND>=1
    SDL_GLContext sdlGlCtx = nullptr;
#endif
    SDL_Window* sdlWindow = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
    bool failedConstruction = false;

#if NCH_GLSDL_OPENGL_BACKEND>=1
    //Re-implemented SDL2 behavior
    GLSDL_Shader* sdrDefault2D = nullptr;
    glm::vec4 renderDrawColor = {0, 0, 0, 1};
    SDL_BlendMode renderDrawBlendMode = SDL_BLENDMODE_NONE;
    void* renderTarget = nullptr;
    int windowW = 1, windowH = 1;
    //Null target cliprect and viewport...
    SDL_Rect ntgtRenderClipRect = {0, 0, 0, 0}; bool ntgtRenderClipEnabled = false;
    SDL_Rect ntgtRenderViewport = {0, 0, 0, 0}; bool ntgtRenderViewportEnabled = false;
    SDL_Rect ntgtEffectiveRenderRect = {0, 0, -1, -1};
    //GL state holders
    static SDL_BlendMode glCachedSDL_BlendMode;
    static SDL_Rect glCachedScissor;
    static glm::mat4 glCachedOrthoMat4;
    static int glCachedApplyTexture;
    static GLfloat prevGL_ClearColor[4];
    static GLint prevGL_Viewport[4];
    static bool prevGL_BlendEnabled;
    static GLint prevGL_BlendSrcRGB, prevGL_BlendDstRGB, prevGL_BlendSrcAlpha, prevGL_BlendDstAlpha;
    static bool prevGL_ScissorTestEnabled; static SDL_Rect prevGL_ScissorBox;
    static bool prevGL_CullFaceEnabled;
    static bool prevGL_DepthTestEnabled;
    static GLint prevGL_ActiveTexture;
    static GLint prevGL_UsedProgram;
#endif
    static bool glSaveStateExists;
};

class GLSDL_Texture {
public:
    GLSDL_Texture(GLSDL_Renderer* glsdlRenderer, uint32_t format, int access, int w, int h);
    GLSDL_Texture(GLSDL_Renderer* glsdlRenderer, SDL_Surface* surface);
    ~GLSDL_Texture();

    bool hasFailedConstruction() const;
    SDL_Texture* toSDL_Texture() const;
    GLuint getGL_TextureID() const;
    GLuint getGL_FramebufferID() const;
    int getGL_TexWidth() const;
    int getGL_TexHeight() const;
    int getSDL_AccessType() const;
    glm::vec4 getSDL_TexColorMod() const;
    SDL_Color getSDL_TexColorModRaw() const;
    SDL_BlendMode getSDL_BlendMode() const;
    uint32_t getSDL_PixelFormat() const;
    int queryTextureAlphaMod(uint8_t* a) const;
    int queryTextureColorMod(uint8_t* r, uint8_t* g, uint8_t* b) const;
    int queryTextureBlendMode(SDL_BlendMode* sdlBlendMode) const;
    int query(uint32_t* format, int* access, int* w, int* h) const;
    SDL_Rect getRenderClipRect() const;
    bool isRenderClipEnabled() const;
    SDL_Rect getRenderViewport() const;
    bool isRenderViewportEnabled() const;
    SDL_Rect getEffectiveRenderRect() const;

    int update(const SDL_Rect* rect, const void* pixels, int pitch);
    int lock(const SDL_Rect* rect, void** pixels, int* pitch);
    void unlock();
    int setColorMod(uint8_t r, uint8_t g, uint8_t b);
    int setAlphaMod(uint8_t a);
    int setBlendMode(SDL_BlendMode sdlBlendMode);
    void setRenderClipRect(const SDL_Rect& rect);
    void setRenderClipEnabled(bool enabled);
    void setRenderViewport(const SDL_Rect& rect);
    void setRenderViewportEnabled(bool enabled);
    void setEffectiveRenderRect(const SDL_Rect& rect);
private:
    void createGL_Texture(uint32_t sdlPixelFormat, int sdlAccessType, int w, int h, bool texFromSurface = false);

    /* Original SDL2 behavior */
#if NCH_GLSDL_OPENGL_BACKEND>=1
#else
    SDL_Renderer* sdlRenderer = nullptr;
    SDL_Texture* sdlTexture = nullptr;
#endif
    bool failedConstruction = false;
    /* Re-implemented SDL2 behavior */
#if NCH_GLSDL_OPENGL_BACKEND>=1
    //Renderer object
    GLSDL_Renderer* glsdlRenderer = nullptr;
    //Texture data features
    GLuint glTexture = 0; int glTexW = 1, glTexH = 1; 
    GLuint glFBO = 0;
    GLuint glPBO = 0; int glPBO_BPP = 4; GLubyte* glPtrPBO = nullptr;
    bool locked = false; SDL_Rect lkRect = {0, 0, 0, 0}; void* lkPixels = nullptr; int lkPitch = 0;
    uint32_t sdlPixelFormat = SDL_PIXELFORMAT_RGBA8888;
    GLenum glTexFormat = GL_RGBA8;
    GLenum glTexUpdateFormat = 0;
    GLenum glTexDataType = GL_UNSIGNED_BYTE;
    int sdlAccessType = SDL_TEXTUREACCESS_STATIC;
    //Texture render features
    glm::vec4 sdlTexColorMod = {1, 1, 1, 1};
    SDL_Color sdlTexColorModRaw = {255, 255, 255, 255};
    SDL_BlendMode sdlBlendMode = SDL_BLENDMODE_NONE;
    SDL_Rect renderClipRect = {0, 0, 0, 0}; bool renderClipEnabled = false;
    SDL_Rect renderViewport = {0, 0, 0, 0}; bool renderViewportEnabled = false;
    SDL_Rect effectiveRenderArea = {0, 0, -1, -1};
#endif
};
#endif