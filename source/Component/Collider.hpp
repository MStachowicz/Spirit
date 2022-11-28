#pragma once

#include "glm/vec3.hpp"

namespace Component
{
    class Collider
    {
    public:
        bool mCollided = false;

        void DrawImGui();
    };
} // namespace Component