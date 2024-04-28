#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Component
{
	struct ViewInformation
	{
		glm::mat4 m_view          = {glm::identity<glm::mat4>()};
		glm::mat4 m_projection    = {glm::identity<glm::mat4>()};
		glm::vec4 m_view_position = {0.f, 0.f, 0.f, 1.f}; // w must be 1.f for position to be transformed correctly.
	};
}