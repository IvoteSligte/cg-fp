#pragma once

#include <SDL2/SDL_events.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
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
