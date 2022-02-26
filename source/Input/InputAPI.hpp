#pragma once

#include "functional"

// Interface used to communicate with Zephyr::Input.
// All subscribe functions must be called before pollEvents
// Input subscribes functions for derived types to callback such as onKeyPress
class InputAPI
{
public:
    enum class Key
    {
        KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

        KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
        KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

        KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,

        KEY_SPACE, KEY_ESCAPE, KEY_ENTER, KEY_TAB,
        KEY_UNKNOWN
    };

    enum class MouseButton
    {
        MOUSE_LEFT, MOUSE_MIDDLE, MOUSE_RIGHT,
        MOUSE_BUTTON_1, MOUSE_BUTTON_2, MOUSE_BUTTON_3, MOUSE_BUTTON_4, MOUSE_BUTTON_5,
        MOUSE_UNKNOWN
    };

    enum class Action
    {
        PRESS, RELEASE, REPEAT,
        UNKNOWN
    };

    InputAPI(std::function<void(const Key&)> pOnKeyPressCallback, std::function<void(const MouseButton&, const Action&)> pOnMousePressCallback, std::function<void(const float&, const float&)> pOnMouseMoveCallback)
    : onKeyPress(pOnKeyPressCallback)
    , onMousePress(pOnMousePressCallback)
    , onMouseMove(pOnMouseMoveCallback)
    {}

    virtual void pollEvents()       = 0;
    virtual bool closeRequested()   = 0;

protected:
    std::function<void(const Key &)> onKeyPress; // The function a derived InputAPI calls
    std::function<void(const MouseButton&, const Action& pAction)> onMousePress;
    std::function<void(const float&, const float&)> onMouseMove; // Called when mouse is moved, supplies the X and Y offset from the last recorded mouse position.
};