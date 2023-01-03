#include "SceneSystem.hpp"

// System
#include "TextureSystem.hpp"
#include "MeshSystem.hpp"

// Component
#include "Camera.hpp"
#include "Collider.hpp"
#include "DirectionalLight.hpp"
#include "Mesh.hpp"
#include "Label.hpp"
#include "PointLight.hpp"
#include "RigidBody.hpp"
#include "SpotLight.hpp"
#include "Transform.hpp"

// Geometry
#include "Geometry.hpp"

namespace System
{
    SceneSystem::SceneSystem(const System::TextureSystem& pTextureSystem, const System::MeshSystem& pMeshSystem)
        : mTextureSystem(pTextureSystem)
        , mMeshSystem(pMeshSystem)
    {
        {
            Component::Label name    = Component::Label("Camera");
            Component::Camera camera = Component::Camera(glm::vec3(0.0f, 1.7f, 7.0f));
            camera.mPrimaryCamera    = true;
            mStorage.addEntity(camera, name);
        }

        {// Cubes
            std::array<glm::vec3, 10> cubePositions = {
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(-1.3f, 0.5f, -1.5f),
                glm::vec3(1.5f, 0.5f, -2.5f),
                glm::vec3(-1.5f, 0.5f, -2.5f),
                glm::vec3(2.4f, 0.5f, -3.5f),
                glm::vec3(1.5f, 0.5f, -5.5f),
                glm::vec3(-1.7f, 0.5f, -7.5f),
                glm::vec3(1.3f, 0.5f, -8.5f),
                glm::vec3(-3.8f, 0.5f, -12.3f),
                glm::vec3(2.0f, 0.5f, -15.0f)};
            for (size_t i = 0; i < 1; i++)
            {
                Component::Transform transform;
                transform.mPosition = cubePositions[i];

                Component::Label name = Component::Label("Cube " + std::to_string(i));

                Component::MeshDraw mesh;
                mesh.mID                = mMeshSystem.getMeshID("cube");
                mesh.mName              = "3DCube";
                mesh.mDrawStyle         = Component::DrawStyle::LightMap;
                mesh.mDiffuseTextureID  = mTextureSystem.getTextureID("metalContainerDiffuse");
                mesh.mSpecularTextureID = mTextureSystem.getTextureID("metalContainerSpecular");
                mesh.mShininess         = 64.f;

                Component::Collider collider;

                Component::RigidBody rigidBody;
                rigidBody.mMass = 72.f;
                rigidBody.mInertiaTensor = Geometry::cuboidInertiaTensor(rigidBody.mMass, 1.f, 1.f, 1.f);
                mStorage.addEntity(mesh, transform, collider, rigidBody, name);
            }
        }
        {// Lights
            {// Point light
                const std::array<glm::vec3, 4> pointLightPositions = {
                    glm::vec3(0.7f, 1.7f, 2.0f),
                    glm::vec3(0.0f, 1.0f, -3.0f),
                    glm::vec3(2.3f, 3.3f, -4.0f),
                    glm::vec3(-4.0f, 2.0f, -12.0f)};
                const std::array<glm::vec3, 4> pointLightColours = {
                    glm::vec3(0.f, 0.f, 1.f),
                    glm::vec3(1.f),
                    glm::vec3(1.f),
                    glm::vec3(1.f)};

                for (size_t i = 0; i < pointLightPositions.size(); i++)
                {
                    Component::Label name = Component::Label("Point light " + std::to_string(i));
                    Component::PointLight pointLight;
                    pointLight.mPosition = pointLightPositions[i];
                    pointLight.mColour   = pointLightColours[i];
                    mStorage.addEntity(pointLight, name);
                }
            }
            {// Directional light
                Component::Label name = Component::Label("Directional light");
                Component::DirectionalLight directionalLight;
                directionalLight.mDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
                mStorage.addEntity(directionalLight, name);
            }
            {// Spotlight
                Component::Label name = Component::Label("Spot light");
                mStorage.addEntity(Component::SpotLight(), name);
            }
        }
    }

    Component::Camera* SceneSystem::getPrimaryCamera()
    {
        Component::Camera* primaryCamera;
        getCurrentScene().foreach([&primaryCamera](Component::Camera& pCamera)
        {
            if (pCamera.mPrimaryCamera)
            {
                primaryCamera = &pCamera;
                return;
            }
        });
        return primaryCamera;
    }
}