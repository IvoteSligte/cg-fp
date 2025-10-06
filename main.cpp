#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

const GLfloat QUAD_VERTICES[] = {
    -1.0, -1.0,
    1.0, -1.0,
    -1.0, 1.0,
    -1.0, 1.0,
    1.0, 1.0,
    1.0, -1.0
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

class GLState {
public:
    GLState()
    {
    }

    bool init()
    {
        std::cout << "Initializing OpenGL." << std::endl;
        // TODO: error handling

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
    }

    void update()
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

private:
    GLuint shaderProgram = 0;
    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;
    GLuint vertexBuffer = 0;
    GLuint vertexArray = 0;
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
        bool running = true;

        while (running) {
            SDL_Event event;
            // go through all events in the queue
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT)
                    running = false;
            }
            glState.update();
            SDL_GL_SwapWindow(window);
        }
    }

private:
    bool initialized = false;
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    GLState glState;
};

int main()
{
    SDLState sdlState;

    if (!sdlState.init())
        return EXIT_FAILURE;

    sdlState.run();

    return EXIT_SUCCESS;
}
