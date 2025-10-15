#include "camera.h"
#include "scene.h"
#include "sdl.h"
#include "shader.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <fstream>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

// #define GLM_ENABLE_EXPERIMENTAL
// #include <glm/gtx/io.hpp>

void glDebugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar* message, const void*)
{
    std::cout << "GL: " << message << std::endl;
}

void setupDebugInfo()
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugCallback, nullptr);
}

class App {
public:
    App()
    {
    }

    void initFullScreenQuad()
    {
        glGenVertexArrays(1, &vertexArray);
        glGenBuffers(1, &vertexBuffer);

        glBindVertexArray(vertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        assert(glIsBuffer(vertexBuffer));
        glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), QUAD_VERTICES, GL_STATIC_DRAW);

        GLint posAttr = 0;
        // each element is 2 times GL_FLOAT since each vec2 is two GL_FLOATs
        glVertexAttribPointer(posAttr, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(posAttr);
    }

    void initChunkBuffer()
    {
        glGenBuffers(1, &storageBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, storageBuffer);
        assert(glIsBuffer(storageBuffer));
        glNamedBufferStorage(storageBuffer, sizeof(chunk), chunk.voxels, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BUFFER_BINDING, storageBuffer);
    }

    bool initShaders()
    {
        // // compute shader
        // {
        //     GLuint lightUpdateShader = loadShader(GL_COMPUTE_SHADER, "light_update.glsl");
        //     if (!voxelProgram.init({ lightUpdateShader }, {})) {
        //         std::cerr << "Failed to initialize OpenGL state (voxelProgram error)." << std::endl;
        //         return false;
        //     }
        // }
        // render shaders
        {
            GLuint vertexShader = loadShader(GL_VERTEX_SHADER, "vert.glsl");
            GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "frag.glsl");
            // the position attribute "inPos" in the vertex shader is set to index 0
            if (!renderProgram.init({ vertexShader, fragmentShader }, { { "inPos", 0 } })) {
                std::cerr << "Failed to initialize OpenGL state (renderProgram error)." << std::endl;
                return false;
            }
        }
        return true;
    }

    bool init()
    {
        chunk.init();

        std::cout << "Initializing OpenGL." << std::endl;
        // TODO: error handling (with glIsBuffers for buffers)

        // --- ensure opengl version 4.3 is used ---
        // TODO:

        setupDebugInfo();
        initFullScreenQuad();
        initChunkBuffer();
        if (!initShaders())
            return false;

        // --- shaders ---
        std::cout << "Finished initializing OpenGL state." << std::endl;
        return true;
    }

    void destroy()
    {
        voxelProgram.destroy();
        renderProgram.destroy();
        if (vertexBuffer)
            glDeleteBuffers(1, &vertexBuffer);
        if (vertexArray)
            glDeleteVertexArrays(1, &vertexArray);
        if (storageBuffer)
            glDeleteBuffers(1, &storageBuffer);
    }

    bool update(InputState& inputs, float deltaTime)
    {
        camera.update(inputs, deltaTime);

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // {
        //     voxelProgram.use();
        //     glDispatchCompute(WORKGROUP_SIZE.x, WORKGROUP_SIZE.y, WORKGROUP_SIZE.z);
        // }
        // // ensure voxel chunk update happens before rendering
        // glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        {
            renderProgram.use();
            glUniform3fv(renderProgram.getUniformLocation("position"), 1, glm::value_ptr(camera.getPosition()));
            glUniformMatrix3fv(renderProgram.getUniformLocation("rotation"), 1, true, glm::value_ptr(glm::inverse(camera.getRotation())));
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        return true;
    }

private:
    Chunk chunk;
    Camera camera;

    ShaderProgram voxelProgram;
    ShaderProgram renderProgram;
    GLuint vertexBuffer = 0;
    GLuint vertexArray = 0;
    GLuint storageBuffer = 0;
};

int main()
{
    SDLState<App> sdlState;

    if (!sdlState.init())
        return EXIT_FAILURE;

    sdlState.run();

    return EXIT_SUCCESS;
}
