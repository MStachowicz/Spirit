#pragma once

#include "InputAPI.hpp" // Included in header because nested enum type InputAPI::Key cannot be forward declared.

class Camera;

// Processes input coming in from a registered mInputHandler using the InputAPI.
class Input
{
public:
    Input(Camera& pCamera);
    void pollEvents();
    bool closeRequested();
private:
    void onInput(const InputAPI::Key& pKeyPressed);
    void onMousePress(const InputAPI::MouseButton& pMouseButton, const InputAPI::Action& pAction);
    void onMouseMove(const float& pXOffset, const float& pYOffset);
    bool mCloseRequested = false;
    bool mCapturingMouse = false;

    Camera& mCurrentCamera;
    InputAPI* mInputHandler = nullptr;
};