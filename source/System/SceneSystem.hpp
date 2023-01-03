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
        SceneSystem(const TextureSystem& pTextureSystem, const MeshSystem& pMeshSystem);
        ECS::Storage& getCurrentScene() { return mStorage; }

        Component::Camera* getPrimaryCamera();

    private:
        void constructBouncingBallScene();
        void constructBoxScene();

        ECS::Storage mStorage;
        const TextureSystem& mTextureSystem;
        const MeshSystem& mMeshSystem;
    };
} // namespace System