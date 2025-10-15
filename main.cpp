#include "app.h"
#include "sdl.h"
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

int main()
{
    SDLState<App> sdlState;

    if (!sdlState.init())
        return EXIT_FAILURE;

    sdlState.run();
    return EXIT_SUCCESS;
}
