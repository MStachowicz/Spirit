#pragma once

#include "Utility/Config.hpp"

#include <stdint.h>

namespace Platform
{
    enum class Key : uint8_t;
    enum class MouseButton;
    enum class Action;
    enum class CursorMode;
    class Window;
    class Input;
}
namespace System
{
    class SceneSystem;

    // Listens to platform input events.
    class InputSystem
    {
    public:
        InputSystem(Platform::Input& p_input, Platform::Window& p_window, SceneSystem& pSceneSystem);
        void update(const DeltaTime& p_delta_time);

        size_t m_update_count;
    private:
        void on_key_event(Platform::Key p_key, Platform::Action p_action);

        Platform::Input& m_input;
        Platform::Window& m_window;

        SceneSystem& mSceneSystem;
    };
}