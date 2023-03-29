#pragma once

#include <stdint.h>

namespace Platform
{
    // Supports 256 Keys
    enum class Key : uint8_t
    {
        Num_0, Num_1, Num_2, Num_3, Num_4, Num_5, Num_6, Num_7, Num_8, Num_9,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        Space, Escape, Enter, Tab,
        Unknown
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

    enum class CursorMode
    {
        NORMAL,   // Cursor is visible and not being captured by the window.
        HIDDEN,   // Cursor is hidden when hovering over the window and not being captured by it.
        CAPTURED, // Cursor is hidden and captured by the window.
        UNKNOWN
    };
}