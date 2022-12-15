#pragma once

#include <vector>

namespace Platform
{
    enum class Key;
    enum class MouseButton;
    enum class Action;
    enum class CursorMode;
}
namespace System
{
    class SceneSystem;
    class CollisionSystem;
}
namespace OpenGL
{
    class OpenGLRenderer;
}
namespace ECS
{
    typedef size_t EntityID;
}

namespace UI
{
    // The engine editor overlaying the window/scene when in non-release mode.
    // Allows editing all entity components via ImGui windows and popups.
    class Editor
    {
    public:
        Editor(System::SceneSystem& pSceneSystem, System::CollisionSystem& pCollisionSystem, OpenGL::OpenGLRenderer& pOpenGLRenderer);

        void draw();

        int mDrawCount;
    private:
        System::SceneSystem& mSceneSystem;
        System::CollisionSystem& mCollisionSystem;
        OpenGL::OpenGLRenderer& mOpenGLRenderer;

        std::vector<ECS::EntityID> mSelectedEntities;

        void onMousePressed(const Platform::MouseButton& pMouseButton, const Platform::Action& pAction);

        void drawEntityPanel();
        void drawGraphicsPanel();
        void drawPerformancePanel();
    };
} // namespace UI