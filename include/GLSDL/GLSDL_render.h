#pragma once
#include "z/GLSDL_render_impl.h"
#ifdef NCH_GLSDL_OPENGL_BACKEND

void GLSDL_GL_SaveState(GLSDL_Renderer* renderer);
void GLSDL_GL_RestoreState(GLSDL_Renderer* renderer);
GLSDL_Renderer* GLSDL_CreateRenderer(GLSDL_Window* window, int index, uint32_t flags);
int GLSDL_RenderClear(GLSDL_Renderer* renderer);
void GLSDL_RenderPresent(GLSDL_Renderer* renderer);
int GLSDL_RenderReadPixels(GLSDL_Renderer* renderer, const SDL_Rect* rect, uint32_t format, void* pixels, int pitch);
int GLSDL_SetRenderDrawColor(GLSDL_Renderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
int GLSDL_GetRenderDrawColor(GLSDL_Renderer* renderer, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);
int GLSDL_SetRenderDrawBlendMode(GLSDL_Renderer* renderer, SDL_BlendMode sdlBlendMode);
int GLSDL_GetRenderDrawBlendMode(GLSDL_Renderer* renderer, SDL_BlendMode* sdlBlendMode);
int GLSDL_RenderSetClipRect(GLSDL_Renderer* renderer, const SDL_Rect* rect);
bool GLSDL_RenderIsClipEnabled(GLSDL_Renderer* renderer);
void GLSDL_RenderGetClipRect(GLSDL_Renderer* renderer, SDL_Rect* rect);
int GLSDL_RenderSetViewport(GLSDL_Renderer* renderer, const SDL_Rect* rect);
void GLSDL_RenderGetViewport(GLSDL_Renderer* renderer, SDL_Rect* rect);
int GLSDL_RenderFillRectF(GLSDL_Renderer* renderer, SDL_FRect* rect);
int GLSDL_RenderFillRect(GLSDL_Renderer* renderer, SDL_Rect* rect);
int GLSDL_RenderDrawRectF(GLSDL_Renderer* renderer, SDL_FRect* rect);
int GLSDL_RenderDrawRect(GLSDL_Renderer* renderer, SDL_Rect* rect);
int GLSDL_RenderDrawLineF(GLSDL_Renderer* renderer, float x1, float y1, float x2, float y2);
int GLSDL_RenderDrawLine(GLSDL_Renderer* renderer, int x1, int y1, int x2, int y2);
int GLSDL_RenderDrawPointF(GLSDL_Renderer* renderer, float x, float y);
int GLSDL_RenderDrawPoint(GLSDL_Renderer* renderer, int x, int y);
int GLSDL_RenderGeometry(GLSDL_Renderer* renderer, GLSDL_Texture* texture, const SDL_Vertex* vertices, int num_vertices, const int* indices, int num_indices);

GLSDL_Texture* GLSDL_CreateTexture(GLSDL_Renderer* glsdlRenderer, uint32_t format, int access, int w, int h);
GLSDL_Texture* GLSDL_CreateTextureFromSurface(GLSDL_Renderer *glsdlRenderer, SDL_Surface *surface);
int GLSDL_UpdateTexture(GLSDL_Texture* texture, const SDL_Rect* rect, const void* pixels, int pitch);
int GLSDL_LockTexture(GLSDL_Texture* texture, const SDL_Rect* rect, void** pixels, int* pitch);
void GLSDL_UnlockTexture(GLSDL_Texture* texture);
void GLSDL_DestroyTexture(GLSDL_Texture* texture);

int GLSDL_RenderCopyExF(GLSDL_Renderer* renderer, GLSDL_Texture* texture, SDL_Rect* srcrect, SDL_FRect* dstrect, double angle, SDL_FPoint* center, SDL_RendererFlip flip);
int GLSDL_RenderCopyEx(GLSDL_Renderer* renderer, GLSDL_Texture* texture, SDL_Rect* srcrect, SDL_Rect* dstrect, double angle, SDL_Point* center, SDL_RendererFlip flip);
int GLSDL_RenderCopyF(GLSDL_Renderer* renderer, GLSDL_Texture* texture, SDL_Rect* srcrect, SDL_FRect* dstrect);
int GLSDL_RenderCopy(GLSDL_Renderer* renderer, GLSDL_Texture* texture, SDL_Rect* srcrect, SDL_Rect* dstrect);
int GLSDL_SetRenderTarget(GLSDL_Renderer* renderer, GLSDL_Texture *texture);
GLSDL_Texture* GLSDL_GetRenderTarget(GLSDL_Renderer* renderer);
int GLSDL_SetTextureColorMod(GLSDL_Texture* texture, uint8_t r, uint8_t g, uint8_t b);
int GLSDL_GetTextureColorMod(GLSDL_Texture* texture, uint8_t* r, uint8_t* g, uint8_t* b);
int GLSDL_SetTextureAlphaMod(GLSDL_Texture* texture, uint8_t a);
int GLSDL_GetTextureAlphaMod(GLSDL_Texture* texture, uint8_t* a);
int GLSDL_SetTextureBlendMode(GLSDL_Texture* texture, SDL_BlendMode sdlBlendMode);
int GLSDL_GetTextureBlendMode(GLSDL_Texture* texture, SDL_BlendMode* sdlBlendMode);
int GLSDL_QueryTexture(GLSDL_Texture* texture, uint32_t* format, int* access, int* w, int* h);

#endif