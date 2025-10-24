#pragma once

#include <GL/glew.h>
#include <string>
#include <vector>

// Loads a shader from the "shaders/" folder, given its name.
GLuint loadShader(GLenum type, const std::string name);

// Loads "common.glsl" and registers it as a named string to be used by #include in shaders.
bool loadCommonShader();

class ShaderProgram {
public:
    ShaderProgram() { };

    bool init(std::initializer_list<GLuint> shaders, std::initializer_list<std::pair<std::string, GLuint>> attributes);
    void destroy();
    void use();
    GLint getUniformLocation(std::string name);

private:
    GLuint program = 0;
    std::vector<GLuint> shaders;
};
