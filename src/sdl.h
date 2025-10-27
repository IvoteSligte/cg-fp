#pragma once

#include "input.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <iostream>

// Assumes App has functions init(), update(InputState& inputs, float deltaTime), destroy()
template <typename App>
class SDLState {
public:
    SDLState()
    {
    }

    // Creates a window and OpenGL context.
    bool init();
    void destroy();
    void run();

private:
    bool initialized = false;
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    InputState inputs;
    App app;
};

inline void logSDLError(std::string msg)
{
    std::cerr << msg << " Error: " << SDL_GetError() << std::endl;
}

// Creates a window and OpenGL context.
template <typename App>
bool SDLState<App>::init()
{
    std::cout << "Initializing SDL." << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        logSDLError("Failed to init SDL.");
        return false;
    }

    // initial width and height
    int width = 800;
    int height = 600;

    window = SDL_CreateWindow("Voxel App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        logSDLError("Failed to create window.");
        return false;
    }
    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    glContext = SDL_GL_CreateContext(window);
    if (glContext == nullptr) {
        logSDLError("Failed to create GL context.");
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

    if (!app.init(width, height))
        return false;

    initialized = true;
    return true;
}

template <typename App>
void SDLState<App>::destroy()
{
    app.destroy();
    if (glContext)
        SDL_GL_DeleteContext(glContext);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}

template <typename App>
void SDLState<App>::run()
{
    const int FRAME_RATE = 3;
    const float FRAME_TIME = 1.0 / FRAME_RATE;

    Uint64 last;
    Uint64 now = SDL_GetPerformanceCounter();
    bool quit = false;

    std::cout << "App started." << std::endl;

    while (!quit) {
        // get deltaTime
        last = now;
        now = SDL_GetPerformanceCounter();
        float deltaTime = (double)((now - last) * 1000) / SDL_GetPerformanceFrequency();

        // get window width/height
        int width;
        int height;
        bool resized = false;
        SDL_GetWindowSize(window, &width, &height);

        // clear inputs for start of frame
        // inputs.held.merge(inputs.pressed); // add pressed to held
        inputs.pressed.clear();
        inputs.mouseDelta = glm::vec2(0.0f);

        SDL_Event event;
        // go through all events in the queue
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                if (!inputs.isHeld(event.key.keysym.sym)) {
                    inputs.pressed.insert(event.key.keysym.sym);
                }
                inputs.held.insert(event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                inputs.pressed.erase(event.key.keysym.sym);
                inputs.held.erase(event.key.keysym.sym);
                break;
            case SDL_MOUSEMOTION:
                inputs.mouseDelta += glm::vec2((float)event.motion.xrel / width, (float)event.motion.yrel / height) * deltaTime;
                break;
            case SDL_WINDOWEVENT_RESIZED:
                width = event.window.data1;
                height = event.window.data2;
                resized = true;
                break;
            case SDL_QUIT:
                quit = true;
                break;
            }
        }

        app.resize(width, height);
        resized = false;
        if (!app.update(inputs, deltaTime))
            break;
        SDL_GL_SwapWindow(window);
    }
}
