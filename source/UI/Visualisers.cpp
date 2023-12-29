#include "Visualisers.hpp"

#include "OpenGL/DebugRenderer.hpp"

#include "Component/Label.hpp"
#include "Component/Mesh.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Transform.hpp"
#include "System/SceneSystem.hpp"
#include "System/MeshSystem.hpp"

#include "Geometry/Frustrum.hpp"
#include "Geometry/Intersect.hpp"

#include "imgui.h"

namespace UI
{
	void draw_frustrum_debugger(float aspect_ratio)
	{
		// Use this ImGui + OpenGL::DebugRenderer function to visualise Projection generated Geometry::Frustrums.
		// A projection-only generated frustrum is positioned at [0, 0, 0] in the positive-z direction.
		// OpenGL clip coordinates are in the [-1 - 1] range thus the default generate ortho projection has a near = -1, far = 1.
		if (ImGui::Begin("Frustrum visualiser"))
		{
			enum class ProjectionType
			{
				Ortho,
				Perspective
			};
			static const std::vector<std::pair<ProjectionType, const char*>> projection_options =
				{{ProjectionType::Ortho, "Ortho"}, {ProjectionType::Perspective, "Perspective"}};
			static ProjectionType projection_type = ProjectionType::Ortho;
			static float near                     = 0.1f;
			static float far                      = 2.f;
			static float ortho_size               = 1.f;
			static bool use_near_far              = true;
			static float fov                      = 90.f;
			static bool transpose                 = false;
			static bool apply_view                = true;

			ImGui::ComboContainer("Projection type", projection_type, projection_options);

			ImGui::Separator();
			glm::mat4 projection;
			if (projection_type == ProjectionType::Ortho)
			{
				ImGui::Checkbox("use near far", &use_near_far);
				if (use_near_far)
				{
					ImGui::Slider("near", near, -1.f, 20.f);
					ImGui::Slider("far", far, 1.f, 20.f);
				}
				ImGui::Slider("ortho_size", ortho_size, 1.f, 20.f);

				if (use_near_far)
					projection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near, far);
				else
					projection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size);
			}
			else if (projection_type == ProjectionType::Perspective)
			{
				ImGui::Slider("FOV", fov, 1.f, 180.f);
				ImGui::Slider("Aspect ratio", aspect_ratio, 0.f, 5.f);
				ImGui::Slider("near", near, -1.f, 20.f);
				ImGui::Slider("far", far, 1.f, 20.f);
				projection = glm::perspective(glm::radians(fov), aspect_ratio, near, far);
			}

			ImGui::Separator();
			ImGui::Checkbox("transpose", &transpose);
			if (transpose)
				projection = glm::transpose(projection);

			ImGui::Checkbox("apply view matrix", &apply_view);
			if (apply_view)
			{
				ImGui::Separator();
				static glm::vec3 eye_position = glm::vec3(0.f, 0.f, 0.f);
				static glm::vec3 center       = glm::vec3(0.5f, 0.f, 0.5f);
				static glm::vec3 up           = glm::vec3(0.f, 1.f, 0.f);
				static glm::mat4 view;
				static bool inverse_view     = false;
				static bool transpose_view   = false;
				static bool swap_order       = false;
				static bool flip_view_dir    = true;
				static bool inverse_position = true;

				ImGui::Slider("Position", eye_position, 0.f, 20.f);
				ImGui::Slider("look direction", center, 0.f, 20.f);
				ImGui::Slider("up direction", up, 0.f, 20.f);
				ImGui::Checkbox("Inverse view", &inverse_view);
				ImGui::Checkbox("Transpose view", &transpose_view);
				ImGui::Checkbox("Swap order", &swap_order);
				ImGui::Checkbox("Flip view direction", &flip_view_dir);
				ImGui::Checkbox("inverse position", &inverse_position);

				glm::vec3 view_position = inverse_position ? -eye_position : eye_position;
				glm::vec3 view_look_at  = flip_view_dir ? view_position - center : view_position + center;
				view                    = glm::lookAt(view_position, view_look_at, up);

				if (swap_order)
				{
					if (inverse_view)
						view = glm::inverse(view);
					if (transpose_view)
						view = glm::transpose(view);
				}
				else
				{
					if (transpose_view)
						view = glm::transpose(view);
					if (inverse_view)
						view = glm::inverse(view);
				}
				projection = projection * view;
				ImGui::Text("VIEW", view);
				ImGui::Separator();
			}

			Geometry::Frustrum frustrum = Geometry::Frustrum(projection);
			ImGui::Text("LEFT  \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_left.m_normal.x, frustrum.m_left.m_normal.y, frustrum.m_left.m_normal.z, frustrum.m_left.m_distance);
			ImGui::Text("RIGHT \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_right.m_normal.x, frustrum.m_right.m_normal.y, frustrum.m_right.m_normal.z, frustrum.m_right.m_distance);
			ImGui::Text("BOTTOM\nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_bottom.m_normal.x, frustrum.m_bottom.m_normal.y, frustrum.m_bottom.m_normal.z, frustrum.m_bottom.m_distance);
			ImGui::Text("TOP   \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_top.m_normal.x, frustrum.m_top.m_normal.y, frustrum.m_top.m_normal.z, frustrum.m_top.m_distance);
			ImGui::Text("NEAR  \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_near.m_normal.x, frustrum.m_near.m_normal.y, frustrum.m_near.m_normal.z, frustrum.m_near.m_distance);
			ImGui::Text("FAR   \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_far.m_normal.x, frustrum.m_far.m_normal.y, frustrum.m_far.m_normal.z, frustrum.m_far.m_distance);
			ImGui::Text("PROJECTION", projection);
			OpenGL::DebugRenderer::add(frustrum, glm::vec4(218.f / 255.f, 112.f / 255.f, 214.f / 255.f, 0.5f));
		}
		ImGui::End();
	}

	void draw_tri_tri_debugger()
	{
		if (ImGui::Begin("Tri Tri visualiser"))
		{
			ImGui::Text("Compare the two triangles and check if they intersect.");

			static Geometry::Triangle t1 = Geometry::Triangle(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 4.f, 0.f), glm::vec3(1.f, 3.f, 0.f));
			static Geometry::Triangle t2 = Geometry::Triangle(glm::vec3(-1.f, 3.f, 1.f), glm::vec3(0.f, 4.f, 1.f), glm::vec3(1.f, 3.f, 1.f));

			ImGui::Slider("Triangle 1 point 1", t1.m_point_1, -10.f, 10.f);
			ImGui::Slider("Triangle 1 point 2", t1.m_point_2, -10.f, 10.f);
			ImGui::Slider("Triangle 1 point 3", t1.m_point_3, -10.f, 10.f);
			ImGui::Separator();
			ImGui::Slider("Triangle 2 point 1", t2.m_point_1, -10.f, 10.f);
			ImGui::Slider("Triangle 2 point 2", t2.m_point_2, -10.f, 10.f);
			ImGui::Slider("Triangle 2 point 3", t2.m_point_3, -10.f, 10.f);

			float shape_alpha = 0.5f;
			glm::vec3 intersected_colour        = glm::vec3(1.f, 0.f, 0.f);
			glm::vec3 not_intersected_colour    = glm::vec3(0.f, 1.f, 0.f);
			glm::vec3 intersection_shape_colour = glm::vec3(1.f, 1.f, 0.f);
			// Base the thickness on the size of the triangles
			float intersection_shape_thickness  = glm::length(t1.centroid() - t1.m_point_2) * 0.01f;
			ImVec4 intersected_colour_imgui     = ImVec4(intersected_colour.r, intersected_colour.g, intersected_colour.b, 1.f);
			ImVec4 not_intersected_colour_imgui = ImVec4(not_intersected_colour.r, not_intersected_colour.g, not_intersected_colour.b, 1.f);

			if (Geometry::intersecting(t1, t2))
			{
				ImGui::TextColored(intersected_colour_imgui, "Triangles intersect");
				OpenGL::DebugRenderer::add(t1, glm::vec4(intersected_colour, shape_alpha));
				OpenGL::DebugRenderer::add(t2, glm::vec4(intersected_colour, shape_alpha));
			}
			else
			{
				ImGui::TextColored(not_intersected_colour_imgui, "Triangles do not intersect");
				OpenGL::DebugRenderer::add(t1, glm::vec4(not_intersected_colour, shape_alpha));
				OpenGL::DebugRenderer::add(t2, glm::vec4(not_intersected_colour, shape_alpha));
			}

			ImGui::Separator();
			bool coplanar = false;
			if (auto optional_line_segment = Geometry::triangle_triangle(t1, t2, &coplanar))
			{
				ImGui::TextColored(intersected_colour_imgui, "Triangles intersect - coplanar: %s", coplanar ? "true" : "false");

				OpenGL::DebugRenderer::add(t1, glm::vec4(intersected_colour, shape_alpha));
				OpenGL::DebugRenderer::add(t2, glm::vec4(intersected_colour, shape_alpha));

				if (!coplanar)
				{
					Geometry::Cylinder cylinder = Geometry::Cylinder(optional_line_segment->m_start, optional_line_segment->m_end, intersection_shape_thickness);
					OpenGL::DebugRenderer::add(cylinder, glm::vec4(intersection_shape_colour, 1.f));
				}
			}
			else
			{
				ImGui::TextColored(not_intersected_colour_imgui, "Triangles do not intersect");
				OpenGL::DebugRenderer::add(t1, glm::vec4(not_intersected_colour, shape_alpha));
				OpenGL::DebugRenderer::add(t2, glm::vec4(not_intersected_colour, shape_alpha));
			}
		}
		ImGui::End();
	}
}