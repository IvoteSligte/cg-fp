#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

// TEMP
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

const glm::vec3 FORWARD = glm::vec3(0.0, 0.0, 1.0);
const glm::vec3 RIGHT = glm::vec3(1.0, 0.0, 0.0);
const glm::vec3 UP = glm::vec3(0.0, 1.0, 0.0);

const GLfloat QUAD_VERTICES[] = {
    -1.0, -1.0,
    1.0, -1.0,
    -1.0, 1.0,
    -1.0, 1.0,
    1.0, 1.0,
    1.0, -1.0
};

// mirrored with frag.glsl
struct Voxel {
    // bit 0 set indicates that the voxel exists
    glm::uint flags;
};

// mirrored with frag.glsl
// size of the voxel chunk in one dimension
const long CHUNK_SIZE = 32;

// mirrored with frag.glsl
const int STORAGE_BUFFER_BINDING = 0;

struct Chunk {
    Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    void init()
    {
        for (uint32_t x = 0; x < CHUNK_SIZE; x++) {
            for (uint32_t y = 0; y < CHUNK_SIZE; y++) {
                for (uint32_t z = 0; z < CHUNK_SIZE; z++) {
                    voxels[x][y][z] = {
                        .flags = 1,
                    };
                }
            }
        }
    }
};

bool readFile(const char* path, std::string& out)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error loading shader: Failed to open file: " << path << std::endl;
        return false;
    }
    std::stringstream sstream;
    sstream << file.rdbuf();
    out = sstream.str();
    return true;
}

// TODO: ensure OpenGL version is at least 4.3 for compute shaders (also put version in shader)

// type is either GL_VERTEX_SHADER, GL_FRAGMENT_SHADER or GL_COMPUTE_SHADER
// path is the path to the shader file
GLuint loadShader(GLenum type, const char* path)
{
    std::string string;
    if (!readFile(path, string)) {
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
        std::cerr << "Error loading shader: " << buf << std::endl;
        return 0; // 0 means invalid OpenGL shader
    }
    return id;
}

void glDebugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar* message, const void*)
{
    std::cout << "GL: " << message << std::endl;
}

struct InputState {
    std::unordered_set<SDL_Keycode> pressed = {};
    std::unordered_set<SDL_Keycode> held = {};
    glm::vec2 mouseDelta = glm::vec2(0.0);

    bool isPressed(SDL_KeyCode key)
    {
        return pressed.find(key) != pressed.end();
    }

    bool isHeld(SDL_KeyCode key)
    {
        return held.find(key) != held.end();
    }
};

class GLState {
public:
    GLState()
    {
    }

    bool init()
    {
        chunk.init();
        position = glm::vec3(0.0);
        rotation = glm::mat3(1.0);

        std::cout << "Initializing OpenGL." << std::endl;
        // TODO: error handling (with gllsBuffers for buffers)

        // --- debug info ---
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugCallback, nullptr);

        // --- full screen quad ---
        glGenVertexArrays(1, &vertexArray);
        glGenBuffers(1, &vertexBuffer);

        glBindVertexArray(vertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), QUAD_VERTICES, GL_STATIC_DRAW);

        GLint posAttr = 0;
        // each element is 2 times GL_FLOAT since each vec2 is two GL_FLOATs
        glVertexAttribPointer(posAttr, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(posAttr);

        // --- storage buffer ---
        glGenBuffers(1, &storageBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, storageBuffer);
        glNamedBufferStorage(storageBuffer, sizeof(chunk), chunk.voxels, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BUFFER_BINDING, storageBuffer);

        // --- shaders ---
        vertexShader = loadShader(GL_VERTEX_SHADER, "vert.glsl");
        fragmentShader = loadShader(GL_FRAGMENT_SHADER, "frag.glsl");

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        // the position attribute "inPos" in the vertex shader is set to index 0
        glBindAttribLocation(shaderProgram, 0, "inPos");
        glLinkProgram(shaderProgram);

        std::cout << "Finished initializing OpenGL state." << std::endl;
        return true;
    }

    ~GLState()
    {
        if (shaderProgram)
            glDeleteProgram(shaderProgram);
        if (vertexShader)
            glDeleteShader(vertexShader);
        if (fragmentShader)
            glDeleteShader(fragmentShader);
        if (vertexBuffer)
            glDeleteBuffers(1, &vertexBuffer);
        if (vertexArray)
            glDeleteVertexArrays(1, &vertexArray);
        if (storageBuffer)
            glDeleteBuffers(1, &storageBuffer);
    }

    void update(InputState& inputs, float deltaTime)
    {
        const float MOVEMENT_SPEED = 1.0;
        const float ROTATE_SPEED = 0.1;
        float moveDelta = MOVEMENT_SPEED * deltaTime;
        float rotateDelta = ROTATE_SPEED * deltaTime;

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        if (inputs.isHeld(SDLK_w)) {
            position += rotation * FORWARD * moveDelta;
        }
        if (inputs.isHeld(SDLK_s)) {
            position += rotation * -FORWARD * moveDelta;
        }
        if (inputs.isHeld(SDLK_d)) {
            position += rotation * RIGHT * moveDelta;
        }
        if (inputs.isHeld(SDLK_a)) {
            position += rotation * -RIGHT * moveDelta;
        }
        rotation = glm::rotate(rotation, inputs.mouseDelta.x * rotateDelta, UP);
        rotation = glm::rotate(rotation, inputs.mouseDelta.y * rotateDelta, RIGHT);

        // std::cout << "Position: " << position << std::endl;
        // std::cout << "Rotation: " << rotation << std::endl;

        // TODO: update position and rotation (using SDL)
        glUseProgram(shaderProgram);
        glUniform3fv(glGetUniformLocation(shaderProgram, "position"), 1, glm::value_ptr(position));
        glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "rotation"), 1, true, glm::value_ptr(glm::mat3_cast(rotation)));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

private:
    Chunk chunk;
    glm::vec3 position;
    glm::quat rotation;

    GLuint shaderProgram = 0;
    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;
    GLuint vertexBuffer = 0;
    GLuint vertexArray = 0;
    GLuint storageBuffer = 0;
};

void log_sdl_error(std::string msg)
{
    std::cerr << msg << " Error: " << SDL_GetError() << std::endl;
}

class SDLState {
public:
    SDLState()
    {
    }

    // Creates a window and OpenGL context.
    bool init()
    {
        std::cout << "Initializing SDL." << std::endl;

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            log_sdl_error("Failed to init SDL.");
            return false;
        }

        // initial width and height (TODO: enable resizing)
        int width = 800;
        int height = 600;

        window = SDL_CreateWindow("Voxel App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (window == nullptr) {
            log_sdl_error("Failed to create window.");
            return false;
        }
        glContext = SDL_GL_CreateContext(window);
        if (glContext == nullptr) {
            log_sdl_error("Failed to create GL context.");
            return false;
        }
        // glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to init GLEW." << std::endl;
            return false;
        }
        std::cout << "Initialization finished.\n"
                  << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

        SDL_SetRelativeMouseMode(SDL_TRUE);

        if (!glState.init())
            return false;

        initialized = true;
        return true;
    }

    ~SDLState()
    {
        glState.~GLState();
        if (glContext)
            SDL_GL_DeleteContext(glContext);
        if (window)
            SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void run()
    {
        Uint64 last;
        Uint64 now = SDL_GetPerformanceCounter();
        bool running = true;

        while (running) {
            // get deltaTime
            last = now;
            now = SDL_GetPerformanceCounter();
            float deltaTime = (double)((now - last) * 1000) / SDL_GetPerformanceFrequency();

            // get window width/height
            int width;
            int height;
            SDL_GetWindowSize(window, &width, &height);

            // clear inputs for start of frame
            inputs.held.merge(inputs.pressed); // add pressed to held
            inputs.pressed.clear();
            inputs.mouseDelta = glm::vec2();

            SDL_Event event;
            // go through all events in the queue
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_KEYDOWN:
                    inputs.pressed.insert(event.key.keysym.sym);
                    break;
                case SDL_KEYUP:
                    inputs.pressed.erase(event.key.keysym.sym);
                    inputs.held.erase(event.key.keysym.sym);
                    break;
                case SDL_MOUSEMOTION:
                    inputs.mouseDelta = glm::vec2((float)event.motion.xrel / width, (float)event.motion.yrel / height) * deltaTime;
                    break;
                case SDL_QUIT:
                    running = false;
                    break;
                }
            }

            glState.update(inputs, deltaTime);
            SDL_GL_SwapWindow(window);
        }
    }

private:
    bool initialized = false;
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    GLState glState;
    InputState inputs;
};

int main()
{
    SDLState sdlState;

    if (!sdlState.init())
        return EXIT_FAILURE;

    sdlState.run();

    return EXIT_SUCCESS;
}
