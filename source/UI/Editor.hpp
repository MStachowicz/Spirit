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

// ImGui extenders.
namespace ImGui
{
    bool ComboContainer(const char* pLabel, const char* pCurrentValue, const std::vector<std::string>& pItems, size_t& outSelectedIndex);
    void Text(const char* pLabel, const glm::vec3& pVec3);
    void Text(const char* pLabel, const glm::mat4& pMat4);
}

namespace UI
{
    // Editor is a Debug-build only overlay for Zephyr that provides a UI for interaction.
    class Editor
    {
        struct Windows
        {
            bool Entity           = false;
            bool Performance      = false;
            bool Graphics         = false;
            bool Physics          = false;
            bool ImGuiDemo        = false;
            bool ImGuiMetrics     = false;
            bool ImGuiStack       = false;
            bool ImGuiAbout       = false;
            bool ImGuiStyleEditor = false;
        };

        System::TextureSystem&   mTextureSystem;
        System::MeshSystem&      mMeshSystem;

        System::SceneSystem&     mSceneSystem;
        System::CollisionSystem& mCollisionSystem;

        OpenGL::OpenGLRenderer&  mOpenGLRenderer;

        std::vector<ECS::Entity> mSelectedEntities;
        Windows mWindowsToDisplay; // All the windows currently being displayed
    public:
        int mDrawCount;

        Editor(System::TextureSystem& pTextureSystem, System::MeshSystem& pMeshSystem, System::SceneSystem& pSceneSystem, System::CollisionSystem& pCollisionSystem, OpenGL::OpenGLRenderer& pOpenGLRenderer);
        void draw();

    private:
        void onMousePressed(const Platform::MouseButton& pMouseButton, const Platform::Action& pAction);
        void drawEntityTreeWindow();
        void drawGraphicsWindow();
        void drawPerformanceWindow();
        void drawPhysicsWindow();
    };
} // namespace UI