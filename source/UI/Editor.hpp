#pragma once

#include "glm/vec3.hpp"
#include "glm/fwd.hpp"

#include <vector>
#include <string>

namespace Platform
{
    enum class Key;
    enum class MouseButton;
    enum class Action;
    enum class CursorMode;
}
namespace System
{
    class TextureSystem;
    class MeshSystem;
    class SceneSystem;
    class CollisionSystem;
}
namespace OpenGL
{
    class OpenGLRenderer;
}
namespace ECS
{
    class Entity;
}

namespace ImGui
{
    bool ComboContainer(const char* pLabel, const char* pCurrentValue, const std::vector<std::string>& pItems, size_t& outSelectedIndex);
    void Text(const char* pLabel, const glm::vec3& pVec3);
    void Text(const char* pLabel, const glm::mat4& pMat4);
}

namespace UI
{
    // The engine editor overlaying the window/scene when in non-release mode.
    // Allows editing all entity components via ImGui windows and popups.
    class Editor
    {
    public:
        Editor(System::TextureSystem& pTextureSystem, System::MeshSystem& pMeshSystem, System::SceneSystem& pSceneSystem, System::CollisionSystem& pCollisionSystem, OpenGL::OpenGLRenderer& pOpenGLRenderer);

        void draw();

        int mDrawCount;
    private:
        System::TextureSystem&   mTextureSystem;
        System::MeshSystem&      mMeshSystem;

        System::SceneSystem&     mSceneSystem;
        System::CollisionSystem& mCollisionSystem;

        OpenGL::OpenGLRenderer&  mOpenGLRenderer;

        std::vector<ECS::Entity> mSelectedEntities;

        void onMousePressed(const Platform::MouseButton& pMouseButton, const Platform::Action& pAction);

        void drawEntityPanel();
        void drawGraphicsPanel();
        void drawPerformancePanel();
    };
} // namespace UI