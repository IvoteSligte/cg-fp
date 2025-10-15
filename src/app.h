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
    Camera camera;

    ShaderProgram voxelProgram;
    ShaderProgram renderProgram;
    GLuint vertexBuffer = 0;
    GLuint vertexArray = 0;
    GLuint storageBuffer = 0;
};
