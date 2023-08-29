#pragma once

#include "Storage.hpp"
#include "Geometry/AABB.hpp"

namespace Component
{
    class Camera;
}

namespace System
{
    class TextureSystem;
    class MeshSystem;

    class Scene
    {
    public:
        ECS::Storage m_entities;
        Geometry::AABB m_bound;

        Component::Camera* get_primary_camera();
    };

    class SceneSystem
    {
        TextureSystem& mTextureSystem;
        MeshSystem& mMeshSystem;

    public:
        Scene m_scene;

        SceneSystem(TextureSystem& pTextureSystem, MeshSystem& pMeshSystem);
        ECS::Storage& getCurrentScene() { return m_scene.m_entities; }
        void update_scene_bounds();

    private:
        void add_default_camera();
        void constructBouncingBallScene();
        void constructBoxScene();
        void primitiveMeshScene();
    };
} // namespace System