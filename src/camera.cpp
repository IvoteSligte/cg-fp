#include "camera.h"
#include <glm/gtc/quaternion.hpp>

void Camera::resize(uint newWidth, uint newHeight)
{
    width = newWidth;
    height = newHeight;
}

void Camera::update(InputState& inputs, float deltaTime)
{
    const float MOVEMENT_SPEED = 0.01;
    const float ROTATE_SPEED = 0.05;
    float moveDelta = MOVEMENT_SPEED * deltaTime;
    float rotateDelta = ROTATE_SPEED * deltaTime;

    // reconstruct rotation
    pitch += -inputs.mouseDelta.y * rotateDelta;
    yaw += -inputs.mouseDelta.x * rotateDelta;
    glm::mat3 rotation = getRotation();
    glm::vec3 right = rotation[0];
    glm::vec3 forward = -rotation[2];
    // FIXME: weird snapping when rotating diagonally (=not along pitch or yaw)

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

glm::vec3 Camera::getPosition()
{
    return position;
}

glm::mat3 Camera::getRotation()
{
    glm::quat yawRotation = glm::angleAxis(yaw, UP);
    glm::vec3 right = yawRotation * RIGHT;
    glm::quat pitchRotation = glm::angleAxis(pitch, right);
    return glm::mat3_cast(glm::normalize(pitchRotation * yawRotation));
}

float Camera::getAspectRatio()
{
    return (float)width / height;
}
