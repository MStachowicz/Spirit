#include "SceneSystem.hpp"

// System
#include "TextureSystem.hpp"
#include "MeshSystem.hpp"

// Component
#include "Camera.hpp"
#include "Collider.hpp"
#include "DirectionalLight.hpp"
#include "Input.hpp"
#include "Label.hpp"
#include "Mesh.hpp"
#include "PointLight.hpp"
#include "RigidBody.hpp"
#include "SpotLight.hpp"
#include "Texture.hpp"
#include "Transform.hpp"

// Geometry
#include "Geometry.hpp"

#include "File.hpp"

namespace System
{
    SceneSystem::SceneSystem(System::TextureSystem& pTextureSystem, System::MeshSystem& pMeshSystem)
        : mStorage{}
        , mTextureSystem(pTextureSystem)
        , mMeshSystem(pMeshSystem)
    {
        add_default_camera();
        primitiveMeshScene();
        //constructBoxScene();
        //constructBouncingBallScene();
    }

    Component::Camera* SceneSystem::getPrimaryCamera()
    {
        Component::Camera* primaryCamera = nullptr;

        getCurrentScene().foreach([&primaryCamera](Component::Camera& pCamera)
        {
            if (pCamera.m_primary)
            {
                primaryCamera = &pCamera;
                return;
            }
        });
        return primaryCamera;
    }

    void SceneSystem::add_default_camera()
    {
        Component::Transform camera_transform;
        camera_transform.mPosition = {0.f, 7.f, 12.5f};
        auto camera = Component::Camera(glm::vec3(0.f, -0.5f, 0.5f), true);
        camera.look_at(glm::vec3(0.f), camera_transform.mPosition);

        mStorage.addEntity(
            camera_transform,
            camera,
            Component::Label("Camera"),
            Component::RigidBody(),
            Component::Input(Component::Input::Camera_Move_Look));
    }

    // Lines up all the available primitive meshes along the x axis with the camera facing them.
    void SceneSystem::primitiveMeshScene()
    {
        constexpr float mesh_count   = 5.f;
        constexpr float mesh_width   = 2.f;
        constexpr float mesh_padding = 1.f;
        constexpr float half_count   = (mesh_count - 1) / 2.f;
        constexpr float start_x      = -half_count * (mesh_width + mesh_padding);
        constexpr float increment    = mesh_width + mesh_padding;

        float running_x = start_x;

        { // Textured cube
            Component::Texture texture;
            texture.mDiffuse  = mTextureSystem.getTexture(Utility::File::textureDirectory / "metalContainerDiffuse.png");
            texture.mSpecular = mTextureSystem.getTexture(Utility::File::textureDirectory / "metalContainerSpecular.png");
            auto transform    = Component::Transform{glm::vec3(running_x, 0.f, -mesh_width)};
            auto mesh         = Component::Mesh{mMeshSystem.mCubePrimitive};

            mStorage.addEntity(
                Component::Label{"Cube"},
                Component::RigidBody{},
                transform,
                mesh,
                Component::Collider{transform, mesh},
                texture);
            running_x += increment;
        }
        { // Cone
            auto transform    = Component::Transform{glm::vec3(running_x, 0.f, -mesh_width)};
            auto mesh         = Component::Mesh{mMeshSystem.mConePrimitive};

            mStorage.addEntity(
                Component::Label{"Cone"},
                Component::RigidBody{},
                transform,
                mesh,
                Component::Collider{transform, mesh});
            running_x += increment;
        }
        { // Cylinder
            auto transform    = Component::Transform{glm::vec3(running_x, 0.f, -mesh_width)};
            auto mesh         = Component::Mesh{mMeshSystem.mCylinderPrimitive};

            mStorage.addEntity(
                Component::Label{"Cylinder"},
                Component::RigidBody{},
                transform,
                mesh,
                Component::Collider{transform, mesh});
            running_x += increment;
        }
        { // Plane/quad
            auto transform    = Component::Transform{glm::vec3(running_x, 0.f, -mesh_width)};
            auto mesh         = Component::Mesh{mMeshSystem.mPlanePrimitive};

            mStorage.addEntity(
                Component::Label{"Plane"},
                Component::RigidBody{},
                transform,
                mesh,
                Component::Collider{transform, mesh});
            running_x += increment;
        }
        { // Sphere
            auto transform    = Component::Transform{glm::vec3(running_x, 0.f, -mesh_width)};
            auto mesh         = Component::Mesh{mMeshSystem.mSpherePrimitive};

            mStorage.addEntity(
                Component::Label{"Sphere"},
                Component::RigidBody{},
                transform,
                mesh,
                Component::Collider{transform, mesh});
            running_x += increment;
        }

        { // Lights
            mStorage.addEntity(Component::Label{"Directional light 1"}, Component::DirectionalLight{});

            mStorage.addEntity(Component::Label{"Point light 1"}, Component::PointLight{});

            { // Red point light in-front of the box.
                auto point_light      = Component::PointLight{};
                point_light.mPosition = glm::vec3(-8.f, 0.f, 1.f);
                point_light.mColour   = glm::vec3(1.f, 0.f, 0.f);
                mStorage.addEntity(Component::Label{"Point light 2"}, point_light);
            }
            { // Spotlight over the box pointing down onto it.
                auto spotlight              = Component::SpotLight{};
                spotlight.mPosition         = glm::vec3(start_x, 3.f, -mesh_width);
                spotlight.mColour           = glm::vec3(0.f, 0.f, 1.f);
                spotlight.mDirection        = glm::vec3(0.f, -.1f, 0.f);
                spotlight.mDiffuseIntensity = 3.f;
                mStorage.addEntity(Component::Label{"Spotlight 1"}, spotlight);
            }
        }
    }

    void SceneSystem::constructBoxScene()
    {
        const auto containerDiffuse  = Utility::File::textureDirectory / "metalContainerDiffuse.png";
        const auto containerSpecular = Utility::File::textureDirectory / "metalContainerSpecular.png";

        {// Cubes
            for (size_t i = 0; i < 100; i += 2)
            {
                Component::Transform transform;
                transform.mPosition = glm::vec3(i, 0.f, 0.f); // cubePositions[i];
                Component::Label name         = Component::Label("Cube " + std::to_string((i / 2) + 1));
                Component::Mesh mesh          = Component::Mesh(mMeshSystem.mCubePrimitive);
                Component::Collider collider  = Component::Collider(transform, mesh);
                Component::Texture texture;
                texture.mDiffuse = mTextureSystem.getTexture(containerDiffuse);
                texture.mSpecular = mTextureSystem.getTexture(containerSpecular);
                Component::RigidBody rigidBody;

                mStorage.addEntity(mesh, transform, collider, rigidBody, name, texture);
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
                directionalLight.mAmbientIntensity = 0.7f;
                directionalLight.mDiffuseIntensity = 0.3f;
                mStorage.addEntity(directionalLight, name);
            }
            {// Spotlight
                Component::Label name = Component::Label("Spot light");
                mStorage.addEntity(Component::SpotLight(), name);
            }
        }
    }
    void SceneSystem::constructBouncingBallScene()
    {
        const auto containerDiffuse  = Utility::File::textureDirectory / "metalContainerDiffuse.png";
        const auto containerSpecular = Utility::File::textureDirectory / "metalContainerSpecular.png";

        { // Ball
            Component::Transform transform;
            transform.mPosition = glm::vec3(-10.f, 5.f, 0.f);

            Component::Label name = Component::Label("Sphere");

            Component::Mesh mesh = Component::Mesh(mMeshSystem.mSpherePrimitive);
            Component::Texture texture;
            texture.mDiffuse = mTextureSystem.getTexture(containerDiffuse);
            texture.mSpecular = mTextureSystem.getTexture(containerSpecular);

            Component::Collider collider = Component::Collider(transform, mesh);

            Component::RigidBody rigidBody;
            rigidBody.mMass = 1.f;
            mStorage.addEntity(mesh, transform, collider, rigidBody, name);
        }
        { // Floor
            Component::Transform transform;
            transform.rotateEulerDegrees(glm::vec3(-90.f, 0.f, 0.f));
            transform.mScale = glm::vec3(50.f);
            Component::Label name = Component::Label("Floor");

            Component::Mesh mesh = {mMeshSystem.mPlanePrimitive};
            Component::Collider collider = Component::Collider(transform, mesh);

            Component::RigidBody rigidBody;
            rigidBody.mMass = 1.f;
            mStorage.addEntity(mesh, transform, collider, rigidBody, name);
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
}