#include "app.h"
#include "scene.h"
#include "shader.h"
#include "util.h"
#include <glm/geometric.hpp>
#include <glm/packing.hpp>

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

void App::initRandomDirections()
{
    for (size_t i = 0; i < RANDOM_DIRECTION_COUNT; i++) {
        randomDirections[i] = glm::packSnorm4x8(glm::vec4(
            glm::normalize(glm::vec3(
                randf_normal(),
                randf_normal(),
                randf_normal())),
            // 0.0f as padding between array elements
            0.0f));
    }
}

void App::initFullScreenQuad()
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

void App::initChunk()
{
    chunk.init();
    glGenBuffers(1, &storageBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, storageBuffer);
    assert(glIsBuffer(storageBuffer));
    glNamedBufferStorage(storageBuffer, sizeof(chunk), chunk.voxels, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BUFFER_BINDING, storageBuffer);
}

bool App::initShaders()
{
    std::cout << "Loading and compiling shaders. This may take a minute." << std::endl;
    // compute shader
    {
        GLuint lightUpdateShader = loadShader(GL_COMPUTE_SHADER, "light_update.glsl");
        if (!voxelProgram.init({ lightUpdateShader }, {})) {
            std::cerr << "Failed to initialize OpenGL state (voxelProgram error)." << std::endl;
            return false;
        }
    }
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
    std::cout << "Finished loading shaders." << std::endl;
    return true;
}

bool App::init(uint width, uint height)
{
    initRandomDirections();
    camera = Camera { glm::vec3(CHUNK_SIZE) / 2.0f, width, height };

    std::cout << "Initializing OpenGL." << std::endl;
    // TODO: error handling (with glIsBuffers for buffers)

    // --- ensure opengl version 4.3 is used ---
    if (!GLEW_VERSION_4_3) {
        std::cerr << "OpenGL version 4.3 is not installed or supported on this computer.\n"
                  << "This application needs it for compute shader support." << std::endl;
        return false;
    }

    setupDebugInfo();
    initFullScreenQuad();
    initChunk();
    if (!initShaders())
        return false;

    // --- shaders ---
    std::cout << "Finished initializing OpenGL state." << std::endl;
    return true;
}

void App::resize(uint newWidth, uint newHeight)
{
    glViewport(0, 0, newWidth, newHeight);
    camera.resize(newWidth, newHeight);
}

void App::destroy()
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

bool App::update(InputState& inputs, float deltaTime)
{
    // recalculate random directions to reduce direction bias
    initRandomDirections();
    // update camera based on user input
    camera.update(inputs, deltaTime);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    {
        voxelProgram.use();
        glUniform1ui(voxelProgram.getUniformLocation("dbColorReadIdx"), dbColorReadIdx);
        glUniform1ui(voxelProgram.getUniformLocation("frameNumber"), frameNumber);
        glUniform1uiv(voxelProgram.getUniformLocation("randomDirections"), RANDOM_DIRECTION_COUNT, randomDirections);
        glDispatchCompute(WORKGROUP_SIZE.x, WORKGROUP_SIZE.y, WORKGROUP_SIZE.z);
    }
    // swap double buffers
    // done before rendering so that the written data from this frame's chunk update is read
    dbColorReadIdx = 1 - dbColorReadIdx;

    // ensure voxel chunk update happens before rendering
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    {
        renderProgram.use();
        glUniform1ui(renderProgram.getUniformLocation("dbColorReadIdx"), dbColorReadIdx);
        glUniform3fv(renderProgram.getUniformLocation("position"), 1, glm::value_ptr(camera.getPosition()));
        glUniformMatrix3fv(renderProgram.getUniformLocation("rotation"), 1, true, glm::value_ptr(glm::inverse(camera.getRotation())));
        glUniform1f(renderProgram.getUniformLocation("aspectRatio"), camera.getAspectRatio());
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    frameNumber += 1;
    return true;
}
