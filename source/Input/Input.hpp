#pragma once

class InputHandler;

namespace InputAPI
{
    enum class Key;
    enum class MouseButton;
    enum class Action;
    enum class CursorMode;
}
namespace Manager
{
    class CameraManager;
}

// Processes input coming in from a registered mInputHandler using the InputAPI.
class Input
{
public:
    Input(Manager::CameraManager& pCameraManager);
    void pollEvents();
    bool closeRequested();
private:
    void onInput(const InputAPI::Key& pKeyPressed);
    void onMousePress(const InputAPI::MouseButton& pMouseButton, const InputAPI::Action& pAction);
    void onMouseMove(const float& pXOffset, const float& pYOffset);
    bool mCloseRequested = false;
    bool mCapturingMouse = false;

    InputHandler* mInputHandler;
    Manager::CameraManager& mCameraManager;
};