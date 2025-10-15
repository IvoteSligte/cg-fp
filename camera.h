#pragma once

#include "input.h"
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

const glm::vec3 FORWARD = glm::vec3(0.0, 0.0, -1.0);
const glm::vec3 RIGHT = glm::vec3(1.0, 0.0, 0.0);
const glm::vec3 UP = glm::vec3(0.0, 1.0, 0.0);

class Camera {
public:
    void update(InputState& inputs, float deltaTime)
    {
        const float MOVEMENT_SPEED = 0.01;
        const float ROTATE_SPEED = 0.01;
        float moveDelta = MOVEMENT_SPEED * deltaTime;
        float rotateDelta = ROTATE_SPEED * deltaTime;

        // reconstruct rotation
        pitch += -inputs.mouseDelta.y * rotateDelta;
        yaw += -inputs.mouseDelta.x * rotateDelta;
        glm::mat3 rotation = getRotation();
        glm::vec3 right = rotation[0];
        glm::vec3 forward = -rotation[2];

        // update position
        if (inputs.isHeld(SDLK_w)) {
            position += forward * moveDelta;
        }
        if (inputs.isHeld(SDLK_s)) {
            position += -forward * moveDelta;
        }
        if (inputs.isHeld(SDLK_d)) {
            position += right * moveDelta;
        }
        if (inputs.isHeld(SDLK_a)) {
            position += -right * moveDelta;
        }

        // std::cout << "Position: " << position << std::endl;
        // std::cout << "Rotation: " << rotation << std::endl;
    }

    glm::vec3 getPosition()
    {
        return position;
    }

    glm::mat3 getRotation()
    {
        glm::quat yawRotation = glm::angleAxis(yaw, UP);
        glm::vec3 right = yawRotation * RIGHT;
        glm::quat pitchRotation = glm::angleAxis(pitch, right);
        return glm::mat3_cast(glm::normalize(pitchRotation * yawRotation));
    }

private:
    glm::vec3 position = glm::vec3(0.0f);
    float pitch = 0.0f;
    float yaw = 0.0f;
};
