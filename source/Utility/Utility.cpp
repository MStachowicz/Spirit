#include "Utility.hpp"

#include "glm/gtc/matrix_transform.hpp"

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

    glm::quat getRotation(const glm::vec3& pStart, const glm::vec3& pDestination)
    {
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