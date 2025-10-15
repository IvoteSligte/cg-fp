#pragma once

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
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct InputState {
    std::unordered_set<SDL_Keycode> pressed = {};
    std::unordered_set<SDL_Keycode> held = {};
    glm::vec2 mouseDelta = glm::vec2(0.0);

    bool isPressed(SDL_Keycode key)
    {
        return pressed.find(key) != pressed.end();
    }

    bool isHeld(SDL_Keycode key)
    {
        return held.find(key) != held.end();
    }
};
