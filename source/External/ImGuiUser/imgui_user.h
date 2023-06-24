// By defining IMGUI_INCLUDE_IMGUI_USER_H in ImUserConfig.hpp this imgui_user.h file is included at the end of imgui.h giving us access to all the defined functions.
// Use this file to extend the ImGui functionality and add support for types.

#pragma once

#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtx/quaternion.hpp"

#include <string>
#include <vector>
#include <algorithm>

// Add extra functions within the ImGui:: namespace here.
namespace ImGui
{
    inline void Text(const char* p_label, const glm::vec3& p_vec3)
    {
        Text("%s: [%.3f, %.3f, %.3f]", p_label, p_vec3[0], p_vec3[1], p_vec3[2]);
    }

    // Display p_quat in WXYZ order
    inline void Text(const char* p_label, const glm::quat& p_quat)
    {
        Text("%s: [%.3f, %.3f, %.3f, %.3f]", p_label, p_quat.w, p_quat.x, p_quat.y, p_quat.z);
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

    inline bool Slider(const char* label, glm::vec3& p_vec3, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
    {
        return SliderFloat3(label, &p_vec3.x, v_min, v_max, format, flags);
    }

    inline bool Slider(const char* label, float& p_float, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
    {
        return SliderFloat(label, &p_float, v_min, v_max, format, flags);
    }


    // Given a list of p_options, create a selectable dropown that returns true on selection and returns the out_selected_index of p_options.
    // Prefer the Typed option when possible, this one is handy when you already have access to a list of stringified p_options.
    inline bool ComboContainer(const char* p_label, const char* p_current_option, const std::vector<std::string>& p_options, size_t& out_selected_option_index)
    {
        if (p_options.empty())
            return false;

        bool result = false;
        if (BeginCombo(p_label, p_current_option))
        {
            for (size_t i = 0; i < p_options.size(); i++)
            {
                if (Selectable(p_options[i].c_str()))
                {
                    out_selected_option_index = i;
                    result = true;
                }
            }
            EndCombo();
        }
        return result;
    }

    // Given a p_current_option and a list of p_options and their labels, creates an ImGui selectable dropdown and assigns the selected option to p_current_option.
    // Returns true if p_current_option has been assigned a new value.
    template <typename Type>
    inline bool ComboContainer(const char* p_label, Type& p_current_option, const std::vector<std::pair<Type, const char*>>& p_options)
    {
        if (p_options.empty())
            return false;

        const auto it = std::find_if(p_options.begin(), p_options.end(), [&p_current_option](const auto& pElement) { return p_current_option == pElement.first; });
        ASSERT(it != p_options.end(), "p_current_option not found in the list p_options, p_options should be a complete list of all types of Type.");

        bool result = false;

        if (BeginCombo(p_label, it->second))
        {
            for (const auto& selectable : p_options)
            {
                auto& [type, label] = selectable;

                if (Selectable(label))
                {
                    p_current_option = type;
                    result = true;
                }
            }
            EndCombo();
        }
        return result;
    }
}