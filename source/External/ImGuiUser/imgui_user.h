// By defining IMGUI_INCLUDE_IMGUI_USER_H in ImUserConfig.hpp this imgui_user.h file is included at the end of imgui.h giving us access to all the defined functions.
// Use this file to extend the ImGui functionality and add support for types.

#pragma once

#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "glm/mat4x4.hpp"

// Add extra functions within the ImGui:: namespace here.
namespace ImGui
{
    inline void Text(const char* p_label, const glm::vec3& p_vec3)
    {
        Text("%s: [%.3f, %.3f, %.3f]", p_label, p_vec3[0], p_vec3[1], p_vec3[2]);
    }

    inline void Text(const char* p_label, const glm::mat4& p_mat4)
    {
        Text("%s:\n[%.3f, %.3f, %.3f, %.3f]\n[%.3f, %.3f, %.3f, %.3f]\n[%.3f, %.3f, %.3f, %.3f]\n[%.3f, %.3f, %.3f, %.3f]"
        , p_label
        , p_mat4[0][0], p_mat4[0][1], p_mat4[0][2], p_mat4[0][3]
        , p_mat4[1][0], p_mat4[1][1], p_mat4[1][2], p_mat4[1][3]
        , p_mat4[2][0], p_mat4[2][1], p_mat4[2][2], p_mat4[2][3]
        , p_mat4[3][0], p_mat4[3][1], p_mat4[3][2], p_mat4[3][3]);
    }
}