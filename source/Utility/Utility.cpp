#include "Utility.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace util
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
}