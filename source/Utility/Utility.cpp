#include "Utility.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include <numbers>

namespace Utility
{
    glm::mat4 GetModelMatrix(const glm::vec3 &pPosition, const glm::vec3 &pRotation, const glm::vec3 &pScale)
    {
        glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), pPosition);
        model = glm::rotate(model, glm::radians(pRotation.x), glm::vec3(1.0, 0.0, 0.0));
        model = glm::rotate(model, glm::radians(pRotation.y), glm::vec3(0.0, 1.0, 0.0));
        model = glm::rotate(model, glm::radians(pRotation.z), glm::vec3(0.0, 0.0, 1.0));
        model = glm::scale(model, pScale);
        return model;
    }

    glm::vec3 toRollPitchYaw(const glm::quat pOrientation)
    {
        glm::dvec3 angles;

        // Roll (x-axis rotation)
        double sinr_cosp = 2.0 * (pOrientation.w * pOrientation.x + pOrientation.y * pOrientation.z);
        double cosr_cosp = 1.0 - 2.0 * (pOrientation.x * pOrientation.x + pOrientation.y * pOrientation.y);
        angles.x = std::atan2(sinr_cosp, cosr_cosp);

        // Pitch (y-axis rotation)
        double sinp = 2 * (pOrientation.w * pOrientation.y - pOrientation.z * pOrientation.x);
        if (std::abs(sinp) >= 1.0)
            angles.y = std::copysign(std::numbers::pi / 2.0, sinp); // use 90 degrees if out of range
        else
            angles.y = std::asin(sinp);

        // Yaw (z-axis rotation)
        double siny_cosp = 2.0 * (pOrientation.w * pOrientation.z + pOrientation.x * pOrientation.y);
        double cosy_cosp = 1.0 - 2.0 * (pOrientation.y * pOrientation.y + pOrientation.z * pOrientation.z);
        angles.z       = std::atan2(siny_cosp, cosy_cosp);

        return glm::vec3(static_cast<float>(angles.x), static_cast<float>(angles.y), static_cast<float>(angles.z));
    }

    glm::quat toQuaternion(const float& pRoll, const float& pPitch, const float& pYaw)
    {
        // Abbreviations for the various angular functions
        // Reference: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

        const float cr = glm::cos(pRoll * 0.5f);
        const float sr = glm::sin(pRoll * 0.5f);
        const float cp = glm::cos(pPitch * 0.5f);
        const float sp = glm::sin(pPitch * 0.5f);
        const float cy = glm::cos(pYaw * 0.5f);
        const float sy = glm::sin(pYaw * 0.5f);

        glm::quat q;
        q.w = cr * cp * cy + sr * sp * sy;
        q.x = sr * cp * cy - cr * sp * sy;
        q.y = cr * sp * cy + sr * cp * sy;
        q.z = cr * cp * sy - sr * sp * cy;
        return q;
    }

    glm::quat getRotation(const glm::vec3& pStart, const glm::vec3& pDestination)
    {
        // Reference: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

        const glm::vec3 start       = normalize(pStart);
        const glm::vec3 destination = normalize(pDestination);

        const float cosTheta = dot(start, destination);
        glm::vec3 rotationAxis;

        if (cosTheta < -1.f + 0.001f)
        { // Special case when vectors in opposite directions, there is no "ideal" rotation axis - guess one; any will do as long as it's perpendicular to start
            rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);

            if (glm::length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
                rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

            rotationAxis = glm::normalize(rotationAxis);
            return glm::angleAxis(glm::radians(180.0f), rotationAxis);
        }
        else
        {
            rotationAxis     = glm::cross(start, destination);
            const float s    = sqrt((1.f + cosTheta) * 2.f);
            const float invs = 1.f / s;
            return glm::quat(s * 0.5f, rotationAxis.x * invs, rotationAxis.y * invs, rotationAxis.z * invs);
        }
    }
}