// By defining IMGUI_INCLUDE_IMGUI_USER_H in ImUserConfig.hpp this imgui_user.h file is included at the end of imgui.h giving us access to all the defined functions.
// Use this file to extend the ImGui functionality and add support for types.

#pragma once

#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtx/quaternion.hpp"

#include "Utility/Logger.hpp"

#include <chrono>
#include <string>
#include <vector>
#include <algorithm>

// Add extra functions within the ImGui:: namespace here.
namespace ImGui
{
	inline void Text_Manual(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		TextV(fmt, args);
		va_end(args);
	}
	inline void Text(const char* p_label, const char* p_string)
	{
		Text("%s: [%s]", p_label, p_string);
	}
	inline void Text(const char* p_label, const std::string& p_string)
	{
		Text(p_label, p_string.c_str());
	}
	inline void Text(const char* p_label, const bool& p_bool)
	{
		Text("%s: [%s]", p_label, p_bool ? "true" : "false");
	}
	inline void Text(const char* p_label, const int& p_int)
	{
		Text("%s: [%d]", p_label, p_int);
	}
	inline void Text(const char* p_label, const unsigned int& p_unsigned_int)
	{
		Text("%s: [%u]", p_label, p_unsigned_int);
	}
	inline void Text(const char* p_label, const unsigned long& p_unsigned_long)
	{
		Text("%s: [%lu]", p_label, p_unsigned_long);
	}
	inline void Text(const char* p_label, const long& p_long)
	{
		Text("%s: [%ld]", p_label, p_long);
	}
	inline void Text(const char* p_label, const long long& p_signed_long_long)
	{
		Text("%s: [%lld]", p_label, p_signed_long_long);
	}
	inline void Text(const char* p_label, const unsigned long long& p_unsigned_long_long)
	{
		Text("%s: [%llu]", p_label, p_unsigned_long_long);
	}
	inline void Text(const char* p_label, const float& p_float)
	{
		Text("%s: [%.3f]", p_label, p_float);
	}
	inline void Text(const char* p_label, const double& p_double)
	{
		Text("%s: [%.3f]", p_label, p_double);
	}
	inline void Text(const char* p_label, const glm::vec2& p_vec2)
	{
		Text("%s: [%.3f, %.3f]", p_label, p_vec2[0], p_vec2[1]);
	}
	inline void Text(const char* p_label, const glm::vec3& p_vec3)
	{
		Text("%s: [%.3f, %.3f, %.3f]", p_label, p_vec3[0], p_vec3[1], p_vec3[2]);
	}
	inline void Text(const char* p_label, const glm::vec4& p_vec4)
	{
		Text("%s: [%.3f, %.3f, %.3f, %.3f]", p_label, p_vec4[0], p_vec4[1], p_vec4[2], p_vec4[3]);
	}
	inline void Text(const char* p_label, const glm::ivec2& p_ivec2)
	{
		Text("%s: [%d, %d]", p_label, p_ivec2[0], p_ivec2[1]);
	}
	inline void Text(const char* p_label, const glm::uvec2& p_uvec2)
	{
		Text("%s: [%u, %u]", p_label, p_uvec2[0], p_uvec2[1]);
	}
	inline void Text(const char* p_label, const glm::ivec3& p_ivec3)
	{
		Text("%s: [%d, %d, %d]", p_label, p_ivec3[0], p_ivec3[1], p_ivec3[2]);
	}
	inline void Text(const char* p_label, const glm::ivec4& p_ivec4)
	{
		Text("%s: [%d, %d, %d, %d]", p_label, p_ivec4[0], p_ivec4[1], p_ivec4[2], p_ivec4[3]);
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
	inline bool Slider(const char* label, glm::vec4& p_vec4, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	{
		return SliderFloat3(label, &p_vec4.x, v_min, v_max, format, flags);
	}
	inline bool Slider(const char* label, float& p_float, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	{
		return SliderFloat(label, &p_float, v_min, v_max, format, flags);
	}
	inline bool Slider(const char* label, int& p_v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
	{
		return SliderInt(label, &p_v, v_min, v_max, format, flags);
	}
	inline bool Slider(const char* label, unsigned int& p_v, unsigned int v_min, unsigned int v_max, const char* format = "%u", ImGuiSliderFlags flags = 0)
	{
		return SliderScalar(label, ImGuiDataType_U32, &p_v, &v_min, &v_max, format, flags);
	}
	template <typename Rep, typename Period>
	bool Slider(const char* label, std::chrono::duration<Rep, Period>& p_v, const std::chrono::duration<Rep, Period>& v_min, const std::chrono::duration<Rep, Period>& v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	{
		Rep count = p_v.count();
		bool modified = Slider(label, count, v_min.count(), v_max.count(), format, flags);
		p_v = std::chrono::duration<Rep, Period>(count);
		return modified;
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