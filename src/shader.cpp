#include "shader.h"
#include <GL/glew.h>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <regex>
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

// TODO: ensure OpenGL version is at least 4.3 for compute shaders

// Replaces `#include "shader.glsl"` with the shader code itself.
bool addIncludes(const std::string& source, std::string& result)
{
    const std::basic_regex regex("#include \".*\"\n");
    auto begin = std::sregex_iterator(source.begin(), source.end(), regex);
    result.clear();
    std::size_t lastPos = 0;

    // TODO: add #line directives
    for (auto it = begin; it != std::sregex_iterator(); ++it) {
        result += source.substr(lastPos, it->position() - lastPos);
        lastPos = it->position() + it->length();
        std::string match = it->str();
        std::string includeSource;
        if (!readShader(match.substr(10, match.length() - 12), includeSource)) {
            return false;
        }
        result += includeSource;
    }
    result.append(source.substr(lastPos));
    return true;
}

// type is either GL_VERTEX_SHADER, GL_FRAGMENT_SHADER or GL_COMPUTE_SHADER
// path is the path to the shader file
GLuint loadShader(GLenum type, const std::string name)
{
    std::cout << "Loading shader " << name << std::endl;
    std::string preIncludeSource;

    if (!readShader(name, preIncludeSource)) {
        return 0; // 0 means invalid OpenGL shader
    }
    std::string source;
    if (!addIncludes(preIncludeSource, source)) {
        return 0;
    }
    const char* rawSource = source.c_str();

    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &rawSource, nullptr);
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
    std::cout << "Successfully loaded shader " << name << std::endl;
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

bool ShaderProgram::init(
    std::initializer_list<GLuint> shaders,
    std::initializer_list<std::pair<std::string, GLuint>> attributes)
{
    std::cout << "Initializing shader program." << std::endl;
    std::cout << "Checking status of shaders." << std::endl;
    for (auto shader : shaders) {
        assert(glIsShader(shader) == GL_TRUE);
        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        assert(compiled == GL_TRUE);
    }
    std::cout << "Creating shader program." << std::endl;
    program = glCreateProgram();
    this->shaders = std::vector(shaders);
    std::cout << "Attaching shaders." << std::endl;
    for (auto shader : shaders) {
        glAttachShader(program, shader);
    }
    std::cout << "Binding attributes." << std::endl;
    for (auto attr : attributes) {
        glBindAttribLocation(program, attr.second, attr.first.c_str());
    }
    std::cout << "Linking shader program." << std::endl;
    glLinkProgram(program);

    std::cout << "Checking linking status." << std::endl;
    // check linking status
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        std::cerr << "Shader program linking error: ";
        reportShaderProgramLog(program);
        return false;
    }
    // NOTE: validation always fails with no error message, despite there being no errors
    // // validate program
    // glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    // if (status == GL_FALSE) {
    //     std::cerr << "Shader program validation error: ";
    //     reportShaderProgramLog(program);
    //     return false;
    // }
    std::cout << "Successfully initialized shader program." << std::endl;
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
