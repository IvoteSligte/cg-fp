#pragma once

#include "input.h"
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>

const glm::vec3 FORWARD = glm::vec3(0.0, 0.0, -1.0);
const glm::vec3 RIGHT = glm::vec3(1.0, 0.0, 0.0);
const glm::vec3 UP = glm::vec3(0.0, 1.0, 0.0);

class Camera {
public:
    Camera(glm::vec3 position)
        : position { position }
    {
    }

    void update(InputState& inputs, float deltaTime);
    glm::vec3 getPosition();
    glm::mat3 getRotation();

private:
    glm::vec3 position = glm::vec3(0.0f);
    float pitch = 0.0f;
    float yaw = 0.0f;
};
