#include "GLSDL_render_impl.h"
#include <glm/ext.hpp>
#include <stdexcept>
#include <iostream>

void GLSDL_Renderer::buildGL_SolidRenderBuffers(GLuint glVAO, GLuint glVBO, GLuint glEBO, const std::vector<GLfloat>& verts, int vStride, const std::vector<GLuint>& indices) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    //Bind the VAO
    glBindVertexArray(glVAO);
    //Bind the VBO and EBO, and upload the data to the GPU
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    //Pos attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vStride*sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    //Color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, vStride*sizeof(GLfloat), (GLvoid*)(2*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    //Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::buildGL_TexedRenderBuffers(GLuint glVAO, GLuint glVBO, GLuint glEBO, const std::vector<GLfloat>& verts, int vStride, const std::vector<GLuint>& indices) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    //Bind the VAO
    glBindVertexArray(glVAO);
    //Bind the VBO and EBO, and upload the data to the GPU
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    //Pos attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vStride*sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    //Color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, vStride*sizeof(GLfloat), (GLvoid*)(2*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    //TexUV attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vStride*sizeof(GLfloat), (GLvoid*)(6*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    //Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::setupRenderTargetSettings(void* renderTarget, bool applyTexture)
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    //Setup shader and uniforms
    sdrUniform2DProjection();
    sdrUniformApplyTexture(applyTexture);
    //Setup framebuffer and viewport
    GLSDL_Texture* rTgt = reinterpret_cast<GLSDL_Texture*>(renderTarget);
    if(rTgt!=nullptr) {
        glBindFramebuffer(GL_FRAMEBUFFER, reinterpret_cast<GLSDL_Texture*>(rTgt)->getGL_FramebufferID());
    }
    glViewport(0, 0, getRenderTargetW(), getRenderTargetH());
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::cleanupRenderTargetSettings(void* renderTarget)
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    GLSDL_Texture* rTgt = reinterpret_cast<GLSDL_Texture*>(renderTarget);
    if(rTgt!=nullptr) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, windowW, windowH);
    }
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}


void GLSDL_Renderer::saveGL_ClearColor() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    glGetFloatv(GL_COLOR_CLEAR_VALUE, prevGL_ClearColor);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::restoreGL_ClearColor() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    glClearColor(
        prevGL_ClearColor[0],
        prevGL_ClearColor[1],
        prevGL_ClearColor[2],
        prevGL_ClearColor[3]
    );
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::saveGL_Viewport() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    glGetIntegerv(GL_VIEWPORT, prevGL_Viewport);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::restoreGL_Viewport() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    glViewport(
        prevGL_Viewport[0],
        prevGL_Viewport[1],
        prevGL_Viewport[2],
        prevGL_Viewport[3]
    );
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::saveGL_BlendFuncState() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    prevGL_BlendEnabled = glIsEnabled(GL_BLEND);
    glGetIntegerv(GL_BLEND_SRC_RGB,   &prevGL_BlendSrcRGB);
    glGetIntegerv(GL_BLEND_DST_RGB,   &prevGL_BlendDstRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &prevGL_BlendSrcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &prevGL_BlendDstAlpha);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
bool GLSDL_Renderer::setGL_BlendFuncState(SDL_BlendMode sdlBlendMode) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(sdlBlendMode==SDL_BLENDMODE_INVALID) return false;
    if(sdlBlendMode==glCachedSDL_BlendMode) {
        //std::cout << "old blending\n";
        return true;
    }
    //std::cout << "new blending\n";
    glCachedSDL_BlendMode = sdlBlendMode;
    
    switch(sdlBlendMode) {
        case SDL_BLENDMODE_NONE: { glBlendFunc(GL_ONE, GL_ZERO); } break;
        case SDL_BLENDMODE_BLEND: {
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        } break;
        case SDL_BLENDMODE_ADD: {
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
        } break;
        case SDL_BLENDMODE_MOD: {
            glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE);
        } break;
        case SDL_BLENDMODE_MUL: {
            glBlendFuncSeparate(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
        } break;
        default: {
            return false;
        } break;
    }

    return true;
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::restoreGL_BlendFuncState() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(prevGL_BlendEnabled) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
    glBlendFuncSeparate(prevGL_BlendSrcRGB, prevGL_BlendDstRGB, prevGL_BlendSrcAlpha, prevGL_BlendDstAlpha);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::saveGL_ScissorState() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    prevGL_ScissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);
    
    GLint sb[4]; glGetIntegerv(GL_SCISSOR_BOX, sb);
    prevGL_ScissorBox = {sb[0], sb[1], sb[2], sb[3]};
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::setGL_ScissorState(GLSDL_Renderer* glsdlRenderer) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    const auto& rtH = glsdlRenderer->getRenderTargetH();
    const auto& eRendRect = glsdlRenderer->getEffectiveRenderTargetRect();

    SDL_Rect scissor = { eRendRect.x, rtH-eRendRect.y-eRendRect.h, eRendRect.w, eRendRect.h };
    if(glsdlRenderer->getRenderTarget()) {
        scissor = { eRendRect.x, eRendRect.y, eRendRect.w, eRendRect.h };
    }
    
    if( scissor.x!=glCachedScissor.x ||
        scissor.y!=glCachedScissor.y ||
        scissor.w!=glCachedScissor.w ||
        scissor.h!=glCachedScissor.h ) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(scissor.x, scissor.y, scissor.w, scissor.h);
        glCachedScissor = scissor;
        //printf("New Scissor: %d %d %d %d\n", scissor.x, scissor.y, scissor.w, scissor.h);
        //std::cout << "new scissor\n";
    } else {
        //std::cout << "old scissor\n";
    }
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif

}
void GLSDL_Renderer::restoreGL_ScissorState() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(prevGL_ScissorTestEnabled) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
    
    const auto& sb = prevGL_ScissorBox;
    glScissor(sb.x, sb.y, sb.w, sb.h);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::saveGL_CullFaceState() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    prevGL_CullFaceEnabled = glIsEnabled(GL_CULL_FACE);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::restoreGL_CullFaceState() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(prevGL_CullFaceEnabled) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::saveGL_DepthTestState() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    prevGL_CullFaceEnabled = glIsEnabled(GL_DEPTH_TEST);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::restoreGL_DepthTestState() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    if(prevGL_DepthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::saveGL_ActiveTextureState() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    glGetIntegerv(GL_ACTIVE_TEXTURE, &prevGL_ActiveTexture);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::restoreGL_ActiveTextureState() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    glActiveTexture(prevGL_ActiveTexture);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::saveGL_UsedProgram() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevGL_UsedProgram);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::restoreGL_UsedProgram() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    glUseProgram(prevGL_UsedProgram);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}

void GLSDL_Renderer::vpUpdate() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    //Update renderer viewport
    if(sdlWindow) {
        SDL_GetWindowSize(sdlWindow, &windowW, &windowH);
        //Fix effective render rect in case window size changes.
        GLSDL_Texture* glsdlTex = reinterpret_cast<GLSDL_Texture*>(renderTarget);
        if(glsdlTex==nullptr) {
            ntgtEffectiveRenderRect = calcEffectiveRenderRect(
                getRenderTargetW(), getRenderTargetH(), ntgtRenderViewport, ntgtRenderViewportEnabled, ntgtRenderClipRect, ntgtRenderClipEnabled
            );
        } else {
            glsdlTex->setEffectiveRenderRect(calcEffectiveRenderRect(
                getRenderTargetW(), getRenderTargetH(),
                glsdlTex->getRenderViewport(), glsdlTex->isRenderViewportEnabled(),
                glsdlTex->getRenderClipRect(), glsdlTex->isRenderClipEnabled()
            ));
        }
    } else {
        throw std::logic_error("nullptr SDL_Window*");
    }

    glViewport(0, 0, windowW, windowH);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::sdrUniform2DProjection(float w, float h) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    //Set orthographic projection for 2D rendering based on viewport width and height.
    glm::mat4 projMat = glm::ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f);
    if(renderTarget!=nullptr) {
        //Must flip pixels upside down if using a render target.
        projMat = glm::ortho(0.0f, w, h, 0.0f, -1.0f, 1.0f);
    }
    if(projMat==glCachedOrthoMat4) {
        return;
    }

    glCachedOrthoMat4 = projMat;
    glUniformMatrix4fv(glGetUniformLocation(sdrDefault2D->getID(), "uProjection"), 1, GL_FALSE, glm::value_ptr(projMat));
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::sdrUniform2DProjection() {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    sdrUniform2DProjection(getRenderTargetW(), getRenderTargetH());
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Renderer::sdrUniformApplyTexture(bool applyTexture) {
#if NCH_GLSDL_OPENGL_BACKEND>=1
    //Set whether to apply texture or not
    if(applyTexture && glCachedApplyTexture==1)  { return; }
    if(!applyTexture && glCachedApplyTexture==0) { return; }
    if(applyTexture) { glCachedApplyTexture = 1; }
    else             { glCachedApplyTexture = 0; }
    
    glUniform1i(glGetUniformLocation(sdrDefault2D->getID(), "uApplyTexture"), static_cast<int>(applyTexture));
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}