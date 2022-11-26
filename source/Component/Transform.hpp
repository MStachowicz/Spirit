#pragma once

#include "glm/vec3.hpp"
#include "glm/gtx/quaternion.hpp"

namespace Component
{
    struct Transform
    {
        glm::vec3 mPosition    = glm::vec3(0.0f); // World space position.
        glm::vec3 mRotation    = glm::vec3(0.0f); // Rotation represented in Euler angles. Range [-360 - 360].
        glm::vec3 mScale       = glm::vec3(1.0f);
        glm::vec3 mDirection   = glm::vec3(0.f, 0.f, 1.f); // Unit vector giving the direction the entity is facing.
        glm::quat mOrientation = glm::identity<glm::quat>(); // Unit quaternion taking the forward direction (0,0,1) to the current orientation.

        void DrawImGui();
    };
}