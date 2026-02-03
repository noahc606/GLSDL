#include "GLSDL_render.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GLSDL_GL_SaveState(GLSDL_Renderer* renderer) {
    renderer->glSaveState(renderer);
}
void GLSDL_GL_RestoreState(GLSDL_Renderer* renderer) {
    renderer->glRestoreState();
}
GLSDL_Renderer* GLSDL_CreateRenderer(GLSDL_Window* window, int index, uint32_t flags) {
    GLSDL_Renderer* ret = new GLSDL_Renderer(window, index, flags);
    if(ret->hasFailedConstruction()) {
        delete ret; ret = nullptr;
    }
    return ret;
}
int GLSDL_RenderClear(GLSDL_Renderer* renderer) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderClear();
}
void GLSDL_RenderPresent(GLSDL_Renderer* renderer) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    renderer->renderPresent();
}
int GLSDL_RenderReadPixels(GLSDL_Renderer* renderer, const SDL_Rect* rect, uint32_t format, void* pixels, int pitch) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderReadPixels(rect, format, pixels, pitch);
}
int GLSDL_SetRenderDrawColor(GLSDL_Renderer *renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->setRenderDrawColor(r, g, b, a);
}
int GLSDL_GetRenderDrawColor(GLSDL_Renderer* renderer, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->getRenderDrawColor(r, g, b, a);
}
int GLSDL_SetRenderDrawBlendMode(GLSDL_Renderer* renderer, SDL_BlendMode sdlBlendMode) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->setRenderDrawBlendMode(sdlBlendMode);
}
int GLSDL_GetRenderDrawBlendMode(GLSDL_Renderer* renderer, SDL_BlendMode* blendMode) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->getRenderDrawBlendMode(blendMode);
}
int GLSDL_RenderSetClipRect(GLSDL_Renderer* renderer, const SDL_Rect* rect) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->setRenderTargetClipRect(rect);
}
bool GLSDL_RenderIsClipEnabled(GLSDL_Renderer* renderer) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->isRenderTargetClipEnabled();
}
void GLSDL_RenderGetClipRect(GLSDL_Renderer* renderer, SDL_Rect* rect) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    *rect = renderer->getRenderTargetClipRect();
}
int GLSDL_RenderSetViewport(GLSDL_Renderer* renderer, const SDL_Rect* rect) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->setRenderTargetViewport(rect);
}
void GLSDL_RenderGetViewport(GLSDL_Renderer* renderer, SDL_Rect* rect) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    *rect = renderer->getRenderTargetViewport();
}
int GLSDL_RenderFillRectF(GLSDL_Renderer* renderer, SDL_FRect* rect) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderFillRectF(rect);
}
int GLSDL_RenderFillRect(GLSDL_Renderer* renderer, SDL_Rect* rect) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderFillRect(rect);
}
int GLSDL_RenderDrawRectF(GLSDL_Renderer* renderer, SDL_FRect* rect) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderDrawRectF(rect);
}
int GLSDL_RenderDrawRect(GLSDL_Renderer* renderer, SDL_Rect* rect) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderDrawRect(rect);
}
int GLSDL_RenderDrawLineF(GLSDL_Renderer* renderer, float x1, float y1, float x2, float y2) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderDrawLineF(x1, y1, x2, y2);
}
int GLSDL_RenderDrawLine(GLSDL_Renderer* renderer, int x1, int y1, int x2, int y2) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderDrawLine(x1, y1, x2, y2);
}
int GLSDL_RenderDrawPointF(GLSDL_Renderer* renderer, float x, float y) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderDrawPointF(x, y);
}
int GLSDL_RenderDrawPoint(GLSDL_Renderer* renderer, int x, int y) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderDrawPoint(x, y);
}
int GLSDL_RenderGeometry(GLSDL_Renderer* renderer, GLSDL_Texture* texture, const SDL_Vertex* vertices, int num_vertices, const int* indices, int num_indices) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderGeometry(texture, vertices, num_vertices, indices, num_indices);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLSDL_Texture* GLSDL_CreateTexture(GLSDL_Renderer* glsdlRenderer, uint32_t format, int access, int w, int h) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    GLSDL_Texture* ret = new GLSDL_Texture(glsdlRenderer, format, access, w, h);
    if(ret->hasFailedConstruction()) {
        delete ret; ret = nullptr;
    }
    return ret;
}
GLSDL_Texture* GLSDL_CreateTextureFromSurface(GLSDL_Renderer* glsdlRenderer, SDL_Surface* surface) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    GLSDL_Texture* ret = new GLSDL_Texture(glsdlRenderer, surface);
    if(ret->hasFailedConstruction()) {
        delete ret; ret = nullptr;
    }
    return ret;
}
int GLSDL_UpdateTexture(GLSDL_Texture* texture, const SDL_Rect* rect, const void* pixels, int pitch) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return texture->update(rect, pixels, pitch);
}
int GLSDL_LockTexture(GLSDL_Texture* texture, const SDL_Rect* rect, void** pixels, int* pitch) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return texture->lock(rect, pixels, pitch);
}
void GLSDL_UnlockTexture(GLSDL_Texture* texture) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    texture->unlock();
}
void GLSDL_DestroyTexture(GLSDL_Texture* texture) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    if(texture!=nullptr) delete texture;
}
int GLSDL_RenderCopyExF(GLSDL_Renderer* renderer, GLSDL_Texture* texture, SDL_Rect* srcrect, SDL_FRect* dstrect, double angle, SDL_FPoint* center, SDL_RendererFlip flip) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderCopyExF(texture, srcrect, dstrect, angle, center, flip);
}
int GLSDL_RenderCopyEx(GLSDL_Renderer* renderer, GLSDL_Texture* texture, SDL_Rect* srcrect, SDL_Rect* dstrect, double angle, SDL_Point* center, SDL_RendererFlip flip) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderCopyEx(texture, srcrect, dstrect, angle, center, flip);
}
int GLSDL_RenderCopyF(GLSDL_Renderer* renderer, GLSDL_Texture* texture, SDL_Rect* srcrect, SDL_FRect* dstrect) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderCopyF(texture, srcrect, dstrect);
}
int GLSDL_RenderCopy(GLSDL_Renderer* renderer, GLSDL_Texture* texture, SDL_Rect* srcrect, SDL_Rect* dstrect) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->renderCopy(texture, srcrect, dstrect);
}
int GLSDL_SetRenderTarget(GLSDL_Renderer* renderer, GLSDL_Texture* texture) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return renderer->setRenderTarget(texture);
}
GLSDL_Texture* GLSDL_GetRenderTarget(GLSDL_Renderer* renderer) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return reinterpret_cast<GLSDL_Texture*>(renderer->getRenderTarget());
}

int GLSDL_SetTextureColorMod(GLSDL_Texture* texture, uint8_t r, uint8_t g, uint8_t b) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return texture->setColorMod(r, g, b);
}
int GLSDL_SetTextureAlphaMod(GLSDL_Texture* texture, uint8_t a) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return texture->setAlphaMod(a);
}
int GLSDL_SetTextureBlendMode(GLSDL_Texture* texture, SDL_BlendMode sdlBlendMode) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return texture->setBlendMode(sdlBlendMode);
}
int GLSDL_GetTextureBlendMode(GLSDL_Texture* texture, SDL_BlendMode* sdlBlendMode) {
    GLSDL_Renderer::assertGL_SaveStateExists();
    return texture->queryTextureBlendMode(sdlBlendMode);
}
int GLSDL_QueryTexture(GLSDL_Texture* texture, uint32_t* format, int* access, int* w, int* h) {
    return texture->query(format, access, w, h);
}
