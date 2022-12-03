#pragma once

class InputHandler;

namespace InputAPI
{
    enum class Key;
    enum class MouseButton;
    enum class Action;
    enum class CursorMode;
}
namespace ECS
{
    class Storage;
}
namespace Component
{
    class Camera;
}

namespace System
{
    // Processes input coming in from a registered mInputHandler using the InputAPI.
    class InputSystem
    {
    public:
        InputSystem(ECS::Storage& pStorage);
        void pollEvents();
        bool closeRequested();
    private:

        void onInput(const InputAPI::Key& pKeyPressed);
        void onMousePress(const InputAPI::MouseButton& pMouseButton, const InputAPI::Action& pAction);
        void onMouseMove(const float& pXOffset, const float& pYOffset);
        bool mCloseRequested = false;
        bool mCapturingMouse = false;
        bool mCapturedThisFrame = false; // If the mouse was captured in this pollEvents cycle.

        static Component::Camera* getPrimaryCamera(ECS::Storage& pStorage);

        InputHandler* mInputHandler;
        ECS::Storage& mStorage;
    };
}