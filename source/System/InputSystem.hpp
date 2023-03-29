#pragma once

#include <stdint.h>

namespace Platform
{
    enum class Key : uint8_t;
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
        void update();

    private:
        void onKeyPressed(const Platform::Key& pKeyPressed);
        void onMousePressed(const Platform::MouseButton& pMouseButton, const Platform::Action& pAction);
        void onMouseMoved(const float& pXOffset, const float& pYOffset);

        SceneSystem& mSceneSystem;
    };
}