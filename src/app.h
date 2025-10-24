#pragma once

#include "camera.h"
#include "scene.h"
#include "shader.h"

class App {
public:
    App()
    {
    }

    void initFullScreenQuad();
    void initChunkBuffer();
    bool initShaders();
    bool init();
    void destroy();
    bool update(InputState& inputs, float deltaTime);

private:
    Chunk chunk;
    Camera camera { glm::vec3(CHUNK_SIZE) / 2.0f };

    ShaderProgram voxelProgram;
    ShaderProgram renderProgram;
    GLuint vertexBuffer = 0;
    GLuint vertexArray = 0;
    GLuint storageBuffer = 0;
    // The index of the color double-buffer.
    GLuint dbColorReadIdx = 0;
};
