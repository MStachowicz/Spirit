#include "Visualisers.hpp"

#include "OpenGL/DebugRenderer.hpp"

#include "Component/Label.hpp"
#include "Component/Mesh.hpp"
#include "Component/RigidBody.hpp"
#include "Component/Transform.hpp"
#include "System/SceneSystem.hpp"

#include "Geometry/GJK.hpp"
#include "Geometry/Frustrum.hpp"
#include "Geometry/Intersect.hpp"

#include "Platform/Core.hpp"

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
			OpenGL::DebugRenderer::add(frustrum, 0.5f);
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

	void draw_GJK_debugger(ECS::Entity& p_entity_1, ECS::Entity& p_entity_2, System::Scene& p_scene, int p_debug_step)
	{
		if (ImGui::Begin("GJK visualiser"))
		{
			ImGui::TextWrapped("Compare if two entities are intersecting by stepping through the GJK algorithm.");

			static float cloud_points_size = 0.01f;
			static float result_point_size = cloud_points_size * 3.f;
			static float line_thickness    = result_point_size * 0.25f;
			{
				ImGui::SeparatorText("Options");
				ImGui::Slider("Cloud points size", cloud_points_size, 0.005f, 0.1f);
				ImGui::Slider("Result point size", result_point_size, 0.005f, 0.1f);
				ImGui::Slider("Line thickness",    line_thickness,    0.005f, 0.1f);
			}

			auto draw_simplex_debug = [&](const GJK::Simplex& p_simplex)
			{
				switch (p_simplex.size)
				{
					case 1: // Draw the point
						ImGui::Text("Simplex is a point");
						OpenGL::DebugRenderer::add(Geometry::Sphere(p_simplex[0], result_point_size), glm::vec4(1.f, 0.f, 0.f, 1.f), 0);
						break;
					case 2: // Draw the points and edge
						ImGui::Text("Simplex is a line");
						OpenGL::DebugRenderer::add(Geometry::Sphere(p_simplex[0], result_point_size), glm::vec4(1.f, 0.f, 0.f, 1.f), 0);
						OpenGL::DebugRenderer::add(Geometry::Sphere(p_simplex[1], result_point_size), glm::vec4(0.f, 1.f, 0.f, 1.f), 0);
						OpenGL::DebugRenderer::add(Geometry::Cylinder(p_simplex[0], p_simplex[1], line_thickness), glm::vec4(0.f, 1.f, 0.f, 1.f));
						break;
					case 3: // Draw the points, edges and face of the triangle
						ImGui::Text("Simplex is a triangle");
						OpenGL::DebugRenderer::add(Geometry::Sphere(p_simplex[0], result_point_size), glm::vec4(1.f, 0.f, 0.f, 1.f), 0);
						OpenGL::DebugRenderer::add(Geometry::Sphere(p_simplex[1], result_point_size), glm::vec4(0.f, 1.f, 0.f, 1.f), 0);
						OpenGL::DebugRenderer::add(Geometry::Sphere(p_simplex[2], result_point_size), glm::vec4(0.f, 0.f, 1.f, 1.f), 0);
						OpenGL::DebugRenderer::add(Geometry::Cylinder(p_simplex[0], p_simplex[1], line_thickness), glm::vec4(0.f, 1.f, 0.f, 1.f));
						OpenGL::DebugRenderer::add(Geometry::Cylinder(p_simplex[0], p_simplex[2], line_thickness), glm::vec4(0.f, 1.f, 0.f, 1.f));
						OpenGL::DebugRenderer::add(Geometry::Cylinder(p_simplex[1], p_simplex[2], line_thickness), glm::vec4(0.f, 1.f, 0.f, 1.f));
						OpenGL::DebugRenderer::add(Geometry::Triangle(p_simplex[0], p_simplex[1], p_simplex[2]), glm::vec4(0.f, 0.8f, 0.f, 0.5f));
						break;
					case 4: // Draw the points, edges and face of the tetrahedron
						ImGui::Text("Simplex is a tetrahedron");
						OpenGL::DebugRenderer::add(Geometry::Sphere(p_simplex[0], result_point_size), glm::vec4(1.f, 0.f, 0.f, 1.f), 0);
						OpenGL::DebugRenderer::add(Geometry::Sphere(p_simplex[1], result_point_size), glm::vec4(0.f, 1.f, 0.f, 1.f), 0);
						OpenGL::DebugRenderer::add(Geometry::Sphere(p_simplex[2], result_point_size), glm::vec4(0.f, 0.f, 1.f, 1.f), 0);
						OpenGL::DebugRenderer::add(Geometry::Sphere(p_simplex[3], result_point_size), glm::vec4(0.f, 1.f, 1.f, 1.f), 0);
						OpenGL::DebugRenderer::add(Geometry::Cylinder(p_simplex[0], p_simplex[1], line_thickness), glm::vec4(0.f, 1.f, 0.f, 1.f));
						OpenGL::DebugRenderer::add(Geometry::Cylinder(p_simplex[0], p_simplex[2], line_thickness), glm::vec4(0.f, 1.f, 0.f, 1.f));
						OpenGL::DebugRenderer::add(Geometry::Cylinder(p_simplex[0], p_simplex[3], line_thickness), glm::vec4(0.f, 1.f, 0.f, 1.f));
						OpenGL::DebugRenderer::add(Geometry::Cylinder(p_simplex[1], p_simplex[2], line_thickness), glm::vec4(0.f, 1.f, 0.f, 1.f));
						OpenGL::DebugRenderer::add(Geometry::Cylinder(p_simplex[1], p_simplex[3], line_thickness), glm::vec4(0.f, 1.f, 0.f, 1.f));
						OpenGL::DebugRenderer::add(Geometry::Cylinder(p_simplex[2], p_simplex[3], line_thickness), glm::vec4(0.f, 1.f, 0.f, 1.f));
						OpenGL::DebugRenderer::add(Geometry::Triangle(p_simplex[0], p_simplex[1], p_simplex[2]), glm::vec4(0.f, 0.8f, 0.f, 0.5f));
						OpenGL::DebugRenderer::add(Geometry::Triangle(p_simplex[0], p_simplex[1], p_simplex[3]), glm::vec4(0.f, 0.8f, 0.f, 0.5f));
						OpenGL::DebugRenderer::add(Geometry::Triangle(p_simplex[0], p_simplex[2], p_simplex[3]), glm::vec4(0.f, 0.8f, 0.f, 0.5f));
						OpenGL::DebugRenderer::add(Geometry::Triangle(p_simplex[1], p_simplex[2], p_simplex[3]), glm::vec4(0.f, 0.8f, 0.f, 0.5f));
						break;
					default:
						break;
				}
			};

			auto entity_1_transform = &p_scene.m_entities.get_component<Component::Transform>(p_entity_1);
			auto entity_2_transform = &p_scene.m_entities.get_component<Component::Transform>(p_entity_2);
			auto entity_1_mesh      = &p_scene.m_entities.get_component<Component::Mesh>(p_entity_1);
			auto entity_2_mesh      = &p_scene.m_entities.get_component<Component::Mesh>(p_entity_2);

			if (entity_1_transform && entity_2_transform && entity_1_mesh && entity_2_mesh && entity_1_mesh->m_mesh && entity_2_mesh->m_mesh)
			{
				{ // Render a debug point cloud of the Minkowski difference.
					ImGui::Separator();
					ImGui::Text("Mesh 1 vertex count", entity_1_mesh->m_mesh->vertex_positions.size());
					ImGui::Text("Mesh 2 vertex count", entity_2_mesh->m_mesh->vertex_positions.size());
					ImGui::Text("Current step", p_debug_step + 1);

					// The GJK algorithm avoids ever doing this by transforming the support point directions into the local space of the objects and transforming the result.
					for (auto& vertex_1 : entity_1_mesh->m_mesh->vertex_positions)
						for (auto& vertex_2 : entity_2_mesh->m_mesh->vertex_positions)
						{
							auto vertex_1_world_space = glm::vec3(entity_1_transform->get_model() * glm::vec4(vertex_1, 1.f));
							auto vertex_2_world_space = glm::vec3(entity_2_transform->get_model() * glm::vec4(vertex_2, 1.f));
							OpenGL::DebugRenderer::add(Geometry::Sphere(vertex_1_world_space - vertex_2_world_space, cloud_points_size), glm::vec4(1.f, 1.f, 1.f, 1.f), 0);
						}
				}

				// Start direction is the vector between the two entities. Improvement would be to use the previous GJK result as the starting direction.
				glm::vec3 direction = glm::normalize(entity_2_transform->m_position - entity_1_transform->m_position);
				GJK::Simplex simplex = {GJK::support_point(direction,
				                                           entity_1_mesh->m_mesh->vertex_positions, entity_1_transform->get_model(), entity_1_transform->m_orientation,
				                                           entity_2_mesh->m_mesh->vertex_positions, entity_2_transform->get_model(), entity_2_transform->m_orientation)};
				direction = -simplex[0]; // AO, search in the direction of the origin. Reversed direction to point towards the origin.

				std::optional<bool> intersecting;
				int step_count = 0;
				GJK::Simplex last_simplex = simplex; // Used to draw the last simplex before the result.

				if (p_debug_step > 0)
				{
					while (true) // Main GJK loop. Converge on a simplex that encloses the origin.
					{
						auto new_support_point = GJK::support_point(direction,
						                                            entity_1_mesh->m_mesh->vertex_positions, entity_1_transform->get_model(), entity_1_transform->m_orientation,
						                                            entity_2_mesh->m_mesh->vertex_positions, entity_2_transform->get_model(), entity_2_transform->m_orientation);

						if (glm::dot(new_support_point, direction) <= 0.f)
						{// If the new support point is not past the origin then its impossible to enclose the origin.
							intersecting = false;
							break;
						}

						last_simplex = simplex;
						// Shift the simplex points along to retain A as the most recently added support point as do_simplex expects.
						simplex.push_front(new_support_point);

						if (++step_count > p_debug_step)
							break; // Stop the loop at the current step.

						if (GJK::do_simplex(simplex, direction))
						{
							intersecting = true;
							break;
						}
					}
				}

				{
					ImGui::Separator();

					if (intersecting)
					{
						draw_simplex_debug(simplex);

						if (*intersecting) ImGui::TextColored(Platform::Core::s_theme.success_text, "The two entities are intersecting.");
						else               ImGui::TextColored(Platform::Core::s_theme.error_text,   "The two entities are not intersecting.");

						auto str = std::format("Took {} steps to converge on the result.", step_count);
						ImGui::Text("%s", str.c_str());

						if (*intersecting)
						{
							auto cp = GJK::EPA(simplex,
											entity_1_mesh->m_mesh->vertex_positions, entity_1_transform->get_model(), entity_1_transform->m_orientation,
											entity_2_mesh->m_mesh->vertex_positions, entity_2_transform->get_model(), entity_2_transform->m_orientation);

							cp.A = glm::vec3(entity_1_transform->get_model() * glm::vec4(cp.A, 1.f));
							cp.B = glm::vec3(entity_2_transform->get_model() * glm::vec4(cp.B, 1.f));

							OpenGL::DebugRenderer::add(Geometry::Sphere(cp.A, result_point_size), glm::vec4(1.f, 0.f, 0.f, 1.f), 0);
							OpenGL::DebugRenderer::add(Geometry::Sphere(cp.B, result_point_size), glm::vec4(0.f, 1.f, 0.f, 1.f), 0);
							OpenGL::DebugRenderer::add(Geometry::Cylinder(cp.A, cp.B, line_thickness), glm::vec4(0.5f, 0.5f, 0.5f, 1.f));

							ImGui::SeparatorText("EPA");
							ImGui::Text("A: [%.3f, %.3f, %.3f]", cp.A.x, cp.A.y, cp.A.z);
							ImGui::Text("B: [%.3f, %.3f, %.3f]", cp.B.x, cp.B.y, cp.B.z);
							ImGui::Text("Normal: [%.3f, %.3f, %.3f]", cp.normal.x, cp.normal.y, cp.normal.z);
							ImGui::Text("Penetration depth: %.3f", cp.penetration_depth);
						}
					}
					else
					{
						draw_simplex_debug(last_simplex);

						static bool draw_direction = true;
						ImGui::Checkbox("Draw direction", &draw_direction);
						if (draw_direction)
						{ // Draw the next point search direction as a plane

							if (last_simplex.size > 0) OpenGL::DebugRenderer::add(Geometry::Ray(last_simplex[0], glm::normalize(direction)));
							if (last_simplex.size > 1) OpenGL::DebugRenderer::add(Geometry::Ray(last_simplex[1], glm::normalize(direction)));
							if (last_simplex.size > 2) OpenGL::DebugRenderer::add(Geometry::Ray(last_simplex[2], glm::normalize(direction)));
							if (last_simplex.size > 3) OpenGL::DebugRenderer::add(Geometry::Ray(last_simplex[3], glm::normalize(direction)));

							ImGui::TextWrapped("Keep stepping over to converge on the GJK result using Left and Right arrows");
						}
					}
				}
			}
		}
		ImGui::End();
	}
}