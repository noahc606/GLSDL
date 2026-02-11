#include "GLSDL_shader.h"
#ifdef NCH_GLSDL_OPENGL_BACKEND
#include <nch/cpp-utils/file-utils.h>
#include <nch/cpp-utils/log.h>
using namespace nch;
GLSDL_Shader::GLSDL_Shader(std::string vtxCode, std::string frgCode)
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    const char* vtxSrc = vtxCode.c_str();
    const char* frgSrc = frgCode.c_str();

    /* Build shader program */ {
        //Vertex shader
        GLuint glVtxShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(glVtxShader, 1, &vtxSrc, NULL);
        glCompileShader(glVtxShader);
        compileErrors(glVtxShader, "VERTEX");

        //Fragment shader
        GLuint glFrgShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(glFrgShader, 1, &frgSrc, NULL);
        glCompileShader(glFrgShader);
        compileErrors(glFrgShader, "FRAGMENT");

        //Create shader program
        id = glCreateProgram();
        glAttachShader(id, glVtxShader);
        glAttachShader(id, glFrgShader);
        glLinkProgram(id);

        //Cleanup (we already have the program by now)
        glDeleteShader(glVtxShader);
        glDeleteShader(glFrgShader);
    }
#else
    throw std::invalid_argument("Constructor only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}

GLSDL_Shader* GLSDL_Shader::readFromFiles(std::string vertexFilePath, std::string fragmentFilePath) {
    std::string vtxCode = FileUtils::readFileContent(vertexFilePath);
    std::string frgCode = FileUtils::readFileContent(fragmentFilePath);
    return new GLSDL_Shader(vtxCode, frgCode);
}
GLSDL_Shader* GLSDL_Shader::readFromAssetPath(std::string assetPath) {
    return readFromFiles(assetPath+".vs", assetPath+".fs");
}
GLSDL_Shader* GLSDL_Shader::createDefault2D_TexOrSolid() {
    return new GLSDL_Shader(
        R"(
            #version 330 core
            layout(location = 0) in vec2 inPos;     //Original position
            layout(location = 1) in vec4 inColor;   //Original color
            layout(location = 2) in vec2 inTexUV;   //Original texture UV
            out vec4 vColor;                        //Final vertex color -> Frag
            out vec2 vTexUV;                        //Final texture UV -> Frag
            uniform mat4 uProjection;               //Projection matrix

            void main() {
                vColor = inColor;
                vTexUV = inTexUV;
                gl_Position = uProjection*vec4(inPos, 0.0, 1.0);
            }
        )",
        R"(
            #version 330 core
            in vec4 vColor;             //Vertex -> Color
            in vec2 vTexUV;             //Vertex -> UV
            out vec4 FragColor;         //Final pixel color
            uniform bool uApplyTexture; //Apply texture?
            uniform sampler2D uTexture;

            void main() {
                if(uApplyTexture) {
                    FragColor = texture(uTexture, vTexUV)*vColor;
                } else {
                    FragColor = vColor;
                }
            }
        )"
    );
}

GLSDL_Shader::~GLSDL_Shader() {
    deleteProgram();
}

GLuint GLSDL_Shader::getID() {
    return id;
}

void GLSDL_Shader::useProgram()
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    glUseProgram(id);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
void GLSDL_Shader::deleteProgram()
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    glDeleteProgram(id);
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}

void GLSDL_Shader::compileErrors(GLuint shader, std::string type)
{
#if NCH_GLSDL_OPENGL_BACKEND>=1
    GLint hasCompiled;
    char infoLog[1024];
    if(type!="PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if(hasCompiled==GL_FALSE) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            Log::errorv(__PRETTY_FUNCTION__, "Shader compilation error", "%s", type.c_str());
        }
    } else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if(hasCompiled==GL_FALSE) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            Log::errorv(__PRETTY_FUNCTION__, "Shader linking error", "%s", type.c_str());
        }
    }
#else
    throw std::invalid_argument("Function only usable with NCH_GLSDL_OPENGL_BACKEND=1");
#endif
}
#endif