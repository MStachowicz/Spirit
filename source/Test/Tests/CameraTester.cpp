#include "CameraTester.hpp"

#include "Component/FirstPersonCamera.hpp"
#include "Component/TwoAxisCamera.hpp"
#include "System/SceneSystem.hpp"
#include "Utility/Logger.hpp"

#include <cmath>

namespace Test
{
	void CameraTester::run_unit_tests()
	{
		{SCOPE_SECTION("Dynamic clipping planes")
			const Geometry::AABB bounds(glm::vec3(-1.f), glm::vec3(1.f));
			const glm::vec3 forward(0.f, 0.f, -1.f);

			const auto distant = Component::ClippingPlanes::fit_to_bounds(bounds, glm::vec3(0.f, 0.f, 10.f), forward);
			CHECK_EQUAL(distant.m_near > 0.f && distant.m_near < 9.f, true, "Near plane stays in front of the scene");
			CHECK_EQUAL(distant.m_far > 11.f && distant.m_far < 20.f, true, "Far plane fits the scene with padding");

			const auto close = Component::ClippingPlanes::fit_to_bounds(bounds, glm::vec3(0.f, 0.f, 1.001f), forward);
			CHECK_EQUAL(close.m_near > 0.f && close.m_near < 0.001f, true, "Close-up geometry is not clipped by the old near plane");
			CHECK_EQUAL(close.m_far > 2.001f, true, "Close-up views retain the back of the scene");

			const auto inside = Component::ClippingPlanes::fit_to_bounds(bounds, glm::vec3(0.f), forward);
			CHECK_EQUAL(inside.m_near > 0.f && inside.m_near < 0.001f, true, "Camera inside bounds keeps a small positive near plane");
			CHECK_EQUAL(inside.m_far > 1.f, true, "Camera inside bounds retains forward geometry");

			const auto zoomed_out = Component::ClippingPlanes::fit_to_bounds(bounds, glm::vec3(0.f, 0.f, 20000000.f), forward);
			CHECK_EQUAL(zoomed_out.m_far > 20000000.f, true, "Zooming out extends beyond the old editor far limit");
			CHECK_EQUAL(zoomed_out.m_near > 1000000.f && zoomed_out.m_near < zoomed_out.m_far, true, "Zooming out also raises near to preserve depth precision");

			const auto orthographic = Component::ClippingPlanes::fit_to_bounds(bounds, glm::vec3(0.f), forward, true);
			CHECK_EQUAL(orthographic.m_near < -1.f && orthographic.m_far > 1.f, true, "Orthographic clipping includes bounds behind the eye");

			const auto behind = Component::ClippingPlanes::fit_to_bounds(bounds, glm::vec3(0.f, 0.f, -10.f), forward);
			CHECK_EQUAL(behind.m_near > 0.f && behind.m_far > behind.m_near, true, "Bounds behind the camera keep a valid perspective range");

			const auto empty = Component::ClippingPlanes::fit_to_bounds(Geometry::AABB(glm::vec3(0.f), glm::vec3(0.f)), glm::vec3(0.f), forward);
			CHECK_EQUAL(empty.m_near > 0.f && empty.m_far > empty.m_near, true, "Empty or point bounds keep a valid perspective range");

			const glm::vec3 eye_position(8.f, 6.f, 10.f);
			const glm::vec3 diagonal = glm::normalize(-eye_position);
			const auto rotated = Component::ClippingPlanes::fit_to_bounds(bounds, eye_position, diagonal);
			for (int corner_index = 0; corner_index < 8; ++corner_index)
			{
				const glm::vec3 corner(
					(corner_index & 1) ? bounds.m_max.x : bounds.m_min.x,
					(corner_index & 2) ? bounds.m_max.y : bounds.m_min.y,
					(corner_index & 4) ? bounds.m_max.z : bounds.m_min.z);
				const float depth = glm::dot(corner - eye_position, diagonal);
				CHECK_EQUAL(depth > rotated.m_near && depth < rotated.m_far, true, "Rotated view retains every scene corner");
			}
		}

		{SCOPE_SECTION("Camera clipping integration")
			const Geometry::AABB bounds(glm::vec3(-1.f), glm::vec3(1.f));
			auto depth_visible = [](const Component::ViewInformation& p_view, const glm::vec3& p_point)
			{
				const glm::vec4 clip = p_view.m_projection * p_view.m_view * glm::vec4(p_point, 1.f);
				return std::isfinite(clip.z) && std::isfinite(clip.w) && clip.w > 0.f && clip.z > -clip.w && clip.z < clip.w;
			};

			Component::FirstPersonCamera first_person;
			const glm::vec3 close_eye(0.f, 0.f, 1.001f);
			first_person.update_clipping_planes(bounds, close_eye);
			CHECK_EQUAL(depth_visible(first_person.view_information(close_eye, 1.f), glm::vec3(0.f, 0.f, 1.f)), true, "First-person projection retains close geometry");

			const glm::vec3 distant_eye(0.f, 0.f, 20000.f);
			first_person.update_clipping_planes(bounds, distant_eye);
			CHECK_EQUAL(first_person.m_far > 20000.f, true, "First-person far plane extends beyond its old limit");
			CHECK_EQUAL(depth_visible(first_person.view_information(distant_eye, 1.f), glm::vec3(0.f)), true, "First-person projection retains distant geometry");

			auto expanded_bounds = bounds;
			expanded_bounds.unite(glm::vec3(0.f, 0.f, -40000.f));
			first_person.update_clipping_planes(expanded_bounds, distant_eye);
			CHECK_EQUAL(first_person.m_far > 60000.f, true, "Expanding scene bounds immediately extends the far plane");
			CHECK_EQUAL(depth_visible(first_person.view_information(distant_eye, 1.f), glm::vec3(0.f, 0.f, -40000.f)), true, "Expanded scene geometry remains visible");
			first_person.update_clipping_planes(bounds, close_eye);
			CHECK_EQUAL(first_person.m_near < 0.001f && first_person.m_far < 3.f, true, "Clipping contracts again after camera motion and bounds shrinkage");

			Component::TwoAxisCamera editor;
			editor.set_orbit_distance(1.001f);
			editor.update_clipping_planes(bounds);
			CHECK_EQUAL(depth_visible(editor.view_information(1.f), glm::vec3(0.f, 0.f, 1.f)), true, "Editor perspective retains close geometry");

			editor.zoom(-250.f);
			editor.update_clipping_planes(bounds);
			CHECK_EQUAL(editor.position().z > 10000000.f, true, "Editor zoom exceeds the old far limit");
			CHECK_EQUAL(depth_visible(editor.view_information(1.f), glm::vec3(0.f)), true, "Editor perspective retains geometry after zooming out");

			editor.set_orthographic(true);
			editor.update_clipping_planes(bounds);
			CHECK_EQUAL(depth_visible(editor.view_information(1.f), glm::vec3(0.f)), true, "Distant orthographic projection retains the scene");

			editor.set_orbit_distance(0.001f);
			editor.update_clipping_planes(bounds);
			CHECK_EQUAL(depth_visible(editor.view_information(1.f), glm::vec3(0.f, 0.f, 1.f)), true, "Orthographic zoom retains geometry behind the eye");
			CHECK_EQUAL(depth_visible(editor.view_information(1.f), glm::vec3(0.f, 0.f, -1.f)), true, "Orthographic zoom retains geometry ahead of the eye");
		}

		{SCOPE_SECTION("Empty scene clipping")
			System::Scene scene;
			scene.m_rendered_bounds = Geometry::AABB(glm::vec3(-100000.f), glm::vec3(100000.f));
			scene.update(1.f);

			CHECK_EQUAL(scene.m_rendered_bounds.m_min, glm::vec3(0.f), "Empty scenes discard old minimum bounds");
			CHECK_EQUAL(scene.m_rendered_bounds.m_max, glm::vec3(0.f), "Empty scenes discard old maximum bounds");

			Component::ViewInformation override_view;
			override_view.m_view_position = glm::vec4(5.f, 5.f, 5.f, 1.f);
			scene.update(1.f, override_view);
			CHECK_EQUAL(scene.m_view_information.m_view_position == override_view.m_view_position, true, "Scene preserves an explicit view override");
		}

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