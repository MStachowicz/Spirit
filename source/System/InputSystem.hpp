#pragma once

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

    // Listens to platform input events.
    class InputSystem
    {
    public:
        InputSystem(SceneSystem& pSceneSystem);

    private:
        void onKeyPressed(const Platform::Key& pKeyPressed);
        void onMousePressed(const Platform::MouseButton& pMouseButton, const Platform::Action& pAction);
        void onMouseMoved(const float& pXOffset, const float& pYOffset);

        SceneSystem& mSceneSystem;
    };
}