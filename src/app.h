#pragma once

#include "camera.h"
#include "scene.h"
#include "shader.h"

// mirrored in light_update.glsl
// NOTE: 512+ causes shader compilation fail
const glm::uint RANDOM_DIRECTION_COUNT = 256;

class App {
public:
    App()
    {
    }

    // Initialize app with window width and height.
    bool init(uint width, uint height);
    // Resize the viewport.
    void resize(uint newWidth, uint newHeight);
    bool update(InputState& inputs, float deltaTime);
    void destroy();

private:
    void initRandomDirections();
    void initFullScreenQuad();
    void initChunkBuffer();
    bool initShaders();

    Chunk chunk;
    Camera camera { glm::vec3(CHUNK_SIZE) / 2.0f, 800, 600 };
    // NOTE: uses vec4 instead of vec3 to ensure 16-byte alignment
    // which is what vec3[] uses in a shader
    glm::vec4 randomDirections[RANDOM_DIRECTION_COUNT];

    ShaderProgram voxelProgram;
    ShaderProgram renderProgram;
    GLuint vertexBuffer = 0;
    GLuint vertexArray = 0;
    GLuint storageBuffer = 0;
    // The index of the color double-buffer.
    GLuint dbColorReadIdx = 0;
    // The number of frames since the start.
    GLuint frameNumber = 0;
};
