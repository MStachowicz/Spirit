#pragma once

#include "Storage.hpp"

namespace Component
{
    class Camera;
}

namespace System
{
    class TextureSystem;
    class MeshSystem;

    class SceneSystem
    {
    public:
        SceneSystem(TextureSystem& pTextureSystem, MeshSystem& pMeshSystem);
        ECS::Storage& getCurrentScene() { return mStorage; }

        Component::Camera* getPrimaryCamera();

    private:
        void add_default_camera();

        void constructBouncingBallScene();
        void constructBoxScene();
        void primitiveMeshScene();

        ECS::Storage mStorage;
        TextureSystem& mTextureSystem;
        MeshSystem& mMeshSystem;
    };
} // namespace System