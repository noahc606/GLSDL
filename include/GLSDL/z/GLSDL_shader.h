#pragma once
#ifdef NCH_GLSDL_OPENGL_BACKEND
#include <GL/glew.h>
#include <string>

class GLSDL_Shader {
public:
    GLSDL_Shader(std::string vertexCode, std::string fragmentCode);
    ~GLSDL_Shader();
    static GLSDL_Shader* readFromFiles(std::string vertexFilePath, std::string fragmentFilePath);
    static GLSDL_Shader* readFromAssetPath(std::string assetPath);
    static GLSDL_Shader* createDefault2D_TexOrSolid();

    GLuint getID();

    void useProgram();
    void deleteProgram();
private:
    void compileErrors(GLuint shader, std::string type);

    GLuint id;
};
#endif