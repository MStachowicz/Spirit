#pragma once

#include "Input.hpp"
#include "EventDispatcher.hpp"

// STD
#include <array>
#include <limits>

namespace Platform
{
    class Window;

    class Core // Working name
    {
    public:
        // INIT
        static void initialise_directories();
        static void initialise_GLFW();
        static void initialise_OpenGL();
        static void initialise_ImGui(const Window& p_window);

        // DEINIT
        // Uninitialise ImGui and GLFW.
        // Make sure no further calls to these happens after cleanup.
        static void cleanup();
    };
}