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
        : mTextureSystem(pTextureSystem)
        , mMeshSystem(pMeshSystem)
    {
        primitiveMeshScene();
        //constructBoxScene();
        //constructBouncingBallScene();
    }

    Component::Camera* SceneSystem::getPrimaryCamera()
    {
        Component::Camera* primaryCamera = nullptr;

        getCurrentScene().foreach([&primaryCamera](Component::Camera& pCamera)
        {
            if (pCamera.m_primary_camera)
            {
                primaryCamera = &pCamera;
                return;
            }
        });
        return primaryCamera;
    }

    // Lines up all the available primitive meshes along the x axis with the camera facing them.
    void SceneSystem::primitiveMeshScene()
    {
        mStorage.addEntity(Component::Camera(true), Component::Input(Component::Input::Move), Component::Label("Camera"));


        constexpr float primitiveCount         = 5.f; // Number of available primitives.
        constexpr float primitiveExtents       = 2.f; // The scale of all the primitives. Primitives are in the object space [-1 - 1].
        constexpr float spaceBetweenPrimitives = 1.f; // Space from the end of one primitive mesh to starts of next.
        constexpr float incrementPerPrimitive  = primitiveExtents + spaceBetweenPrimitives;
        float runningXPosition = -((primitiveCount / 2.f) * incrementPerPrimitive) - (primitiveExtents);
        auto containerDiffuse  = Utility::File::textureDirectory / "metalContainerDiffuse.png";
        auto containerSpecular = Utility::File::textureDirectory / "metalContainerSpecular.png";
        auto directions        = Utility::File::textureDirectory / "directions.jpg";

        {
            Component::Transform transform;
            transform.mPosition          = glm::vec3(runningXPosition += incrementPerPrimitive, 0.f, 0.f);
            Component::Label name        = Component::Label("cube");

            Component::Texture texture;
            texture.mDiffuse  = mTextureSystem.getTexture(containerDiffuse);
            texture.mSpecular = mTextureSystem.getTexture(containerSpecular);

            Component::Mesh mesh         = {mMeshSystem.mCubePrimitive};
            Component::Collider collider = Component::Collider(transform, mesh);
            Component::RigidBody rigidBody;
            mStorage.addEntity(mesh, transform, collider, rigidBody, name, texture);
        }
        {
            Component::Transform transform;
            transform.mPosition   = glm::vec3(runningXPosition += incrementPerPrimitive, 0.f, 0.f);
            Component::Label name = Component::Label("cube 2");

            Component::Texture texture;
            texture.mDiffuse = mTextureSystem.getTexture(containerDiffuse);
            texture.mSpecular = mTextureSystem.getTexture(containerSpecular);

            Component::Mesh mesh         = {mMeshSystem.mCubePrimitive};
            Component::Collider collider = Component::Collider(transform, mesh);
            Component::RigidBody rigidBody;
            mStorage.addEntity(mesh, transform, collider, rigidBody, name, texture);
        }
        //{
        //    Component::Transform transform;
        //    transform.mPosition          = glm::vec3(runningXPosition += incrementPerPrimitive, 0.f, 0.f);
        //    Component::Label name        = Component::Label("cone");
        //    Component::Mesh mesh         = {mMeshSystem.mConePrimitive};
        //    Component::Collider collider = Component::Collider(transform, mesh);
        //    Component::RigidBody rigidBody;
        //    mStorage.addEntity(mesh, transform, collider, rigidBody, name);
        //}
        //{
        //    Component::Transform transform;
        //    transform.mPosition          = glm::vec3(runningXPosition += incrementPerPrimitive, 0.f, 0.f);
        //    Component::Label name        = Component::Label("cylinder");
        //    Component::Mesh mesh         = {mMeshSystem.mCylinderPrimitive};
        //    Component::Collider collider = Component::Collider(transform, mesh);
        //    Component::RigidBody rigidBody;
        //    mStorage.addEntity(mesh, transform, collider, rigidBody, name);
        //}
        //{
        //    Component::Transform transform;
        //    transform.mPosition          = glm::vec3(runningXPosition += incrementPerPrimitive, 0.f, 0.f);
        //    Component::Label name        = Component::Label("plane");
        //    Component::Texture texture;
        //    texture.mDiffuse = mTextureSystem.mTextureManager.getOrCreate([&directions](const Data::Texture& pTexture) { return pTexture.mFilePath == directions; }, directions);
        //    Component::Mesh mesh         = {mMeshSystem.mPlanePrimitive};
        //    Component::Collider collider = Component::Collider(transform, mesh);
        //    Component::RigidBody rigidBody;
        //    mStorage.addEntity(mesh, transform, collider, rigidBody, texture, name);
        //}
        //{
        //    Component::Transform transform;
        //    transform.mPosition          = glm::vec3(runningXPosition += incrementPerPrimitive, 0.f, 0.f);
        //    Component::Label name        = Component::Label("sphere");
        //    Component::Mesh mesh         = {mMeshSystem.mSpherePrimitive};
        //    Component::Collider collider = Component::Collider(transform, mesh);
        //    Component::RigidBody rigidBody;
        //    mStorage.addEntity(mesh, transform, collider, rigidBody, name);
        //}
    }

    void SceneSystem::constructBoxScene()
    {
        const auto containerDiffuse  = Utility::File::textureDirectory / "metalContainerDiffuse.png";
        const auto containerSpecular = Utility::File::textureDirectory / "metalContainerSpecular.png";

        mStorage.addEntity(Component::Camera(true), Component::Label("Camera"));

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

        mStorage.addEntity(Component::Camera(true), Component::Label("Camera"));

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