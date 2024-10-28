#include "CameraTester.hpp"

#include "Component/FirstPersonCamera.hpp"
#include "Utility/Logger.hpp"

namespace Test
{
	void CameraTester::run_unit_tests()
	{
		SCOPE_SECTION("FirstPersonCamera")
		{
			constexpr float EPSILON = 0.0001f;
			Component::FirstPersonCamera camera;
			camera.m_pitch = 0.f;
			camera.m_yaw   = 0.f;
			camera.m_far   = 100.f;
			{SCOPE_SECTION("Max view distance")
				{SCOPE_SECTION("Square Aspect Ratio")
					const float square_aspect_ratio = 1.0f;

					camera.m_vertical_FOV = glm::radians(120.f);
					CHECK_EQUAL_FLOAT(camera.get_maximum_view_distance(square_aspect_ratio), 264.575134f, "120-degree FOV", EPSILON);

					camera.m_vertical_FOV = glm::radians(90.f);
					CHECK_EQUAL_FLOAT(camera.get_maximum_view_distance(square_aspect_ratio), 173.205093f, "90-degree FOV", EPSILON);

					camera.m_vertical_FOV = glm::radians(60.f);
					CHECK_EQUAL_FLOAT(camera.get_maximum_view_distance(square_aspect_ratio), 129.099442f, "60-degree FOV", EPSILON);
				}// Square Aspect Ratio

				{SCOPE_SECTION("Wide Aspect Ratio")
					const float wide_aspect_ratio = 2.0f;

					camera.m_vertical_FOV = glm::radians(120.f);
					CHECK_EQUAL_FLOAT(camera.get_maximum_view_distance(wide_aspect_ratio), 400.f, "120-degree FOV", EPSILON);

					camera.m_vertical_FOV = glm::radians(90.f);
					CHECK_EQUAL_FLOAT(camera.get_maximum_view_distance(wide_aspect_ratio), 244.94899f, "90-degree FOV", EPSILON);

					camera.m_vertical_FOV = glm::radians(60.f);
					CHECK_EQUAL_FLOAT(camera.get_maximum_view_distance(wide_aspect_ratio), 163.299316f, "60-degree FOV", EPSILON);
				}// Wide Aspect Ratio

				{SCOPE_SECTION("Narrow Aspect Ratio")
					const float narrow_aspect_ratio = 0.5f;

					camera.m_vertical_FOV = glm::radians(120.f);
					CHECK_EQUAL_FLOAT(camera.get_maximum_view_distance(narrow_aspect_ratio), 217.944962f, "120-degree FOV", EPSILON);

					camera.m_vertical_FOV = glm::radians(90.f);
					CHECK_EQUAL_FLOAT(camera.get_maximum_view_distance(narrow_aspect_ratio), 150.f, "90-degree FOV", EPSILON);

					camera.m_vertical_FOV = glm::radians(60.f);
					CHECK_EQUAL_FLOAT(camera.get_maximum_view_distance(narrow_aspect_ratio), 119.023811f, "60-degree FOV", EPSILON);
				}// Narrow Aspect Ratio
			}// Max view distance

			{SCOPE_SECTION("FOV getters")
				camera.m_vertical_FOV = glm::radians(90.f);

				CHECK_EQUAL_FLOAT(camera.get_horizontal_FOV(1.0f), glm::radians(90.f), "Square aspect ratio", EPSILON);
				CHECK_EQUAL_FLOAT(camera.get_horizontal_FOV(2.0f), glm::radians(126.869901337f), "Wide aspect ratio", EPSILON);
				CHECK_EQUAL_FLOAT(camera.get_horizontal_FOV(0.5f), glm::radians(53.1301041f), "Narrow aspect ratio", EPSILON);
			}
		}
	}
} // namespace Test