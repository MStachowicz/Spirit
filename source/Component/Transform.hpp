#pragma once

#include "glm/gtx/quaternion.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

namespace Component
{
    struct Transform
    {
        inline static const glm::vec3 StartingDirection = glm::vec3(0.f, 0.f, 1.f);

        glm::vec3 mPosition      = glm::vec3(0.0f);   // World-space position.
        glm::vec3 mRollPitchYaw  = glm::vec3(0.0f);   // Roll, Pitch, Yaw rotation represented in Euler degree angles. Range [-180 - 180].
        glm::vec3 mScale         = glm::vec3(1.0f);
        glm::vec3 mDirection     = StartingDirection; // World-space direction vector the entity is facing.
        glm::quat mOrientation   = glm::identity<glm::quat>(); // Unit quaternion taking the StartingDirection to the current orientation.

        glm::mat4 mModel = glm::identity<glm::mat4>();

        // Rotate the object to roll pitch and yaw euler angles in the order XYZ. Angles suppled are in degrees.
        void rotateEulerDegrees(const glm::vec3& pRollPitchYawDegrees);

        void DrawImGui();
    };
}