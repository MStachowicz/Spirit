#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Component
{
	struct ViewInformation
	{
		glm::mat4 m_view          = {glm::identity<glm::mat4>()};
		glm::vec3 m_view_position = {glm::vec3{0.f}};
		glm::mat4 m_projection    = {glm::identity<glm::mat4>()};
	};
}