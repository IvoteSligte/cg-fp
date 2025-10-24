#include "shader.h"
#include <GL/glew.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static const std::string SHADER_FOLDER = std::string(PROJECT_ROOT) + "/shaders/";

bool readShader(const std::string name, std::string& out)
{
    std::ifstream file(SHADER_FOLDER + name);
    if (!file.is_open()) {
        std::cerr << "Error loading shader " << name << ": Failed to open file." << std::endl;
        return false;
    }
    std::stringstream sstream;
    sstream << file.rdbuf();
    out = sstream.str();
    return true;
}

bool loadCommonShader()
{
    // NOTE: name needs to start with a / according to ARB_shading_language_include
    std::string name = "/common.glsl";
    std::string source;
    if (!readShader(name, source)) {
        return false;
    }
    assert(source.length() > 0);
    glNamedStringARB(GL_SHADER_INCLUDE_ARB, name.length(), name.c_str(), (GLint)source.length(), source.c_str());
    assert(glIsNamedStringARB(name.length(), name.c_str()));
    return true;
}

// TODO: ensure OpenGL version is at least 4.3 for compute shaders (also put version in shader)

// type is either GL_VERTEX_SHADER, GL_FRAGMENT_SHADER or GL_COMPUTE_SHADER
// path is the path to the shader file
GLuint loadShader(GLenum type, const std::string name)
{
    std::string string;

    if (!readShader(name, string)) {
        return 0; // 0 means invalid OpenGL shader
    }
    const char* source = string.c_str();

    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);

    GLint compiled = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        const int BUF_SIZE = 1024;
        char buf[BUF_SIZE];
        glGetShaderInfoLog(id, BUF_SIZE, nullptr, buf);
        std::cerr << "Error loading shader " << name << ": " << buf << std::endl;
        return 0; // 0 means invalid OpenGL shader
    }
    assert(glIsShader(id));
    return id;
}

void reportShaderProgramLog(GLuint shaderProgram)
{
    const int BUF_SIZE = 1024;
    char buf[BUF_SIZE];
    int buf_size = 0;
    glGetProgramInfoLog(shaderProgram, BUF_SIZE, &buf_size, buf);
    if (buf_size == 0)
        std::cerr << "[empty program info log]" << std::endl;
    else
        std::cerr << buf << std::endl;
}

bool ShaderProgram::init(std::initializer_list<GLuint> shaders, std::initializer_list<std::pair<std::string, GLuint>> attributes)
{
    for (auto shader : shaders) {
        assert(glIsShader(shader) == GL_TRUE);
        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        assert(compiled == GL_TRUE);
    }
    program = glCreateProgram();
    this->shaders = std::vector(shaders);
    for (auto shader : shaders) {
        glAttachShader(program, shader);
    }
    for (auto attr : attributes) {
        glBindAttribLocation(program, attr.second, attr.first.c_str());
    }
    glLinkProgram(program);

    // check linking status
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        std::cerr << "Shader program linking error: ";
        reportShaderProgramLog(program);
        return false;
    }
    // // validate program
    // glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    // if (status == GL_FALSE) {
    //     std::cerr << "Shader program validation error: ";
    //     reportShaderProgramLog(program);
    //     return false;
    // }
    return true;
}

void ShaderProgram::destroy()
{
    // TODO: need glDetachShader ?
    if (glIsProgram(program)) {
        glDeleteProgram(program);
    }
    for (auto shader : shaders) {
        if (glIsShader(shader)) {
            glDeleteShader(shader);
        }
    }
}

void ShaderProgram::use()
{
    glUseProgram(program);
}

GLint ShaderProgram::getUniformLocation(std::string name)
{
    return glGetUniformLocation(program, name.c_str());
}
