#include "glm/vec3.hpp"
#include "glm/trigonometric.hpp"

struct DirectionalLight
{
    glm::vec3 mDirection      = glm::vec3(0.f, 0.f, 1.f);
    glm::vec3 mColour         = glm::vec3(1.f, 1.f, 1.f);
    float mAmbientIntensity   = 0.05f;
    float mDiffuseIntensity   = 0.15f;
    float mSpecularIntensity  = 0.5f;
};
struct PointLight
{
    glm::vec3 mPosition      = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 mColour        = glm::vec3(1.f, 1.f, 1.f);
    float mAmbientIntensity  = 0.05f;
    float mDiffuseIntensity  = 0.8f;
    float mSpecularIntensity = 1.0f;

    float mConstant          = 1.f;
    float mLinear            = 0.09f;
    float mQuadratic         = 0.032f;
};
struct SpotLight
{
    glm::vec3 mPosition   = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 mDirection  = glm::vec3(0.f, 0.f, -1.f);
    glm::vec3 mColour     = glm::vec3(1.f, 1.f, 1.f);
    float mAmbientIntensity   = 0.0f;
    float mDiffuseIntensity   = 1.0f;
    float mSpecularIntensity  = 1.0f;

    float mConstant     = 1.f;
    float mLinear       = 0.09f;
    float mQuadratic    = 0.032f;

    float mCutOff       = glm::cos(glm::radians(12.5f));
    float mOuterCutOff  = glm::cos(glm::radians(15.0f));
};