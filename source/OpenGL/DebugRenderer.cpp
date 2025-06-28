#include "DebugRenderer.hpp"
#include "OpenGL/GLState.hpp"
#include "OpenGL/DrawCall.hpp"

#include "Component/Collider.hpp"
#include "Component/Lights.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"
#include "System/SceneSystem.hpp"

#include "Geometry/Cone.hpp"
#include "Geometry/Cuboid.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/Frustrum.hpp"
#include "Geometry/Geometry.hpp"
#include "Geometry/Intersect.hpp"
#include "Geometry/LineSegment.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Point.hpp"
#include "Geometry/Quad.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Triangle.hpp"

#include "Utility/Performance.hpp"

namespace OpenGL
{
	static constexpr float Z_Far_Scaler = 1000.f; // Scale the geometry that extends infinitely by this to give it an infinite appearance.

	DebugRenderer::DebugOptions DebugRenderer::m_debug_options{};

	void DebugRenderer::init()
	{
		m_debug_shader          = {"DebugRender"};
		m_bound_shader          = {"uniformColour"};
		m_light_position_shader = {"light_position"};

		{// Create a cube meshes to represent AABBs.
			{
				auto mb = Utility::MeshBuilder<Data::PositionVertex, PrimitiveMode::Lines>{};
				mb.add_cuboid(Geometry::Cuboid(glm::vec3(0.f), glm::vec3(0.5f)));
				m_AABB_outline_mesh = mb.get_mesh();
			}
			{
				auto mb = Utility::MeshBuilder<Data::PositionVertex, PrimitiveMode::Triangles>{};
				mb.add_cuboid(Geometry::Cuboid(glm::vec3(0.f), glm::vec3(0.5f)));
				m_AABB_filled_mesh = mb.get_mesh();
			}
		}
		{// Make the point light mesh
			auto mb = Utility::MeshBuilder<Data::PositionVertex, OpenGL::PrimitiveMode::Triangles>{};
			mb.add_icosphere(glm::vec3(0.f), 1.f, 1);
			m_point_light_mesh = mb.get_mesh();
		}
	}
	void DebugRenderer::deinit()
	{
		m_debug_shader.reset();
	}
	void DebugRenderer::clear()
	{
		m_line_mb.clear();
		m_tri_mb.clear();
	}
	void DebugRenderer::render(System::SceneSystem& p_scene, const Buffer& p_view_properties, const Buffer& p_point_lights_buffer, const FBO& p_target_FBO)
	{
		PERF(DebugRendererRender);
		if (!m_line_mb.empty())
		{
			auto line_mesh = m_line_mb.get_mesh();
			DrawCall dc;
			dc.m_cull_face_enabled = false;
			dc.m_blending_enabled  = line_mesh.has_alpha;
			dc.set_UBO("ViewProperties", p_view_properties);
			dc.submit(m_debug_shader.value(), line_mesh.get_VAO(), p_target_FBO);
		}
		if (!m_tri_mb.empty())
		{
			auto tri_mesh = m_tri_mb.get_mesh();
			DrawCall dc;
			dc.m_cull_face_enabled = false;
			dc.m_blending_enabled  = tri_mesh.has_alpha;
			dc.set_UBO("ViewProperties", p_view_properties);
			dc.submit(m_debug_shader.value(), tri_mesh.get_VAO(), p_target_FBO);
		}

		auto& scene = p_scene.get_current_scene_entities();
		auto& opt = m_debug_options;

		if (opt.m_show_bounding_box)
		{
			scene.foreach([&](Component::Collider& p_collider)
			{
				auto model = glm::translate(glm::identity<glm::mat4>(), p_collider.m_world_AABB.get_center());
				model = glm::scale(model, p_collider.m_world_AABB.get_size());
				{
					DrawCall dc;
					dc.m_cull_face_enabled      = false;
					dc.m_polygon_offset_enabled = true;
					dc.m_polygon_offset_factor  = opt.m_position_offset_factor;
					dc.m_polygon_offset_units   = opt.m_position_offset_units;
					dc.set_uniform("model", model);
					dc.set_uniform("colour", p_collider.m_collided ? glm::vec4(opt.m_bounding_box_collided_colour, 1.f) : glm::vec4(opt.m_bounding_box_colour, 1.f));
					dc.set_UBO("ViewProperties", p_view_properties);
					dc.submit(*m_bound_shader, m_AABB_outline_mesh->get_VAO(), p_target_FBO);
				}
				if (opt.m_fill_bounding_box)
				{
					DrawCall dc;
					dc.m_blending_enabled       = true;
					dc.m_cull_face_enabled      = false;
					dc.m_polygon_offset_enabled = true;
					dc.m_polygon_offset_factor  = opt.m_position_offset_factor;
					dc.m_polygon_offset_units   = opt.m_position_offset_units;
					dc.set_uniform("model", model);
					dc.set_uniform("colour", p_collider.m_collided ? glm::vec4(opt.m_bounding_box_collided_colour, 0.2f) : glm::vec4(opt.m_bounding_box_colour, 0.2f));
					dc.set_UBO("ViewProperties", p_view_properties);
					dc.submit(*m_bound_shader, m_AABB_filled_mesh->get_VAO(), p_target_FBO);
				}
			});
		}

		if (opt.m_show_light_positions)
		{
			GLsizei point_light_count = static_cast<GLsizei>(scene.count_components<Component::PointLight>());
			if (point_light_count > 0)
			{
				DrawCall dc;
				dc.set_uniform("scale", opt.m_light_position_scale);
				dc.set_UBO("ViewProperties", p_view_properties);
				dc.set_SSBO("PointLightsBuffer", p_point_lights_buffer);
				dc.submit_instanced(*m_light_position_shader, m_point_light_mesh->get_VAO(), p_target_FBO, point_light_count);
			}
		}
	}

	void DebugRenderer::add(const Geometry::Triangle& p_triangle, const glm::vec4& p_colour)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_triangle(p_triangle);
	}
	void DebugRenderer::add_axes(const glm::vec3& p_point, float length)
	{
		float scale = 0.01f * length;
		m_tri_mb.set_colour(glm::vec4(1.f, 0.f, 0.f, 1.f));
		m_tri_mb.add_cylinder(p_point, p_point + glm::vec3(length, 0.f, 0.f), scale, m_debug_options.m_segments);
		m_tri_mb.set_colour(glm::vec4(0.f, 1.f, 0.f, 1.f));
		m_tri_mb.add_cylinder(p_point, p_point + glm::vec3(0.f, length, 0.f), scale, m_debug_options.m_segments);
		m_tri_mb.set_colour(glm::vec4(0.f, 0.f, 1.f, 1.f));
		m_tri_mb.add_cylinder(p_point, p_point + glm::vec3(0.f, 0.f, length), scale, m_debug_options.m_segments);
	}
	void DebugRenderer::add(const Geometry::Cone& p_cone, const glm::vec4& p_colour, size_t segments)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_cone(p_cone.m_base, p_cone.m_top, p_cone.m_base_radius, segments);
	}
	void DebugRenderer::add(const Geometry::Cylinder& p_cylinder, const glm::vec4& p_colour, size_t segments)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_cylinder(p_cylinder.m_base, p_cylinder.m_top, p_cylinder.m_radius, segments);
	}
	void DebugRenderer::add(const Geometry::Cuboid& p_cuboid, const glm::vec4& p_colour)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_cuboid(p_cuboid);
	}
	void DebugRenderer::add(const Geometry::Quad& p_quad, const glm::vec4& p_colour)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_quad(p_quad);
	}
	void DebugRenderer::add(const Geometry::LineSegment& p_line, const glm::vec4& p_colour)
	{
		m_line_mb.set_colour(p_colour);
		m_line_mb.add_line(p_line);
	}
	void DebugRenderer::add(const Geometry::Ray& p_ray, const glm::vec4& p_colour)
	{
		// Because a Ray extends infinitely, we represent it as a LineSegment extending beyond camera z-far which gives it an infinite appearance.
		m_line_mb.set_colour(p_colour);
		m_line_mb.add_line(p_ray.m_start, p_ray.m_start + (p_ray.m_direction * Z_Far_Scaler));
	}
	void DebugRenderer::add(const Geometry::Sphere& p_sphere, const glm::vec4& p_colour, size_t subdivisions)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_icosphere(p_sphere.m_center, p_sphere.m_radius, subdivisions);
	}
	void DebugRenderer::add(const Geometry::Frustrum& frustrum, float alpha)
	{
		auto near_top_left     = Geometry::get_intersection(frustrum.m_near, frustrum.m_top,    frustrum.m_left);
		auto near_top_right    = Geometry::get_intersection(frustrum.m_near, frustrum.m_top,    frustrum.m_right);
		auto near_bottom_left  = Geometry::get_intersection(frustrum.m_near, frustrum.m_bottom, frustrum.m_left);
		auto near_bottom_right = Geometry::get_intersection(frustrum.m_near, frustrum.m_bottom, frustrum.m_right);
		auto far_top_left      = Geometry::get_intersection(frustrum.m_far,  frustrum.m_top,    frustrum.m_left);
		auto far_top_right     = Geometry::get_intersection(frustrum.m_far,  frustrum.m_top,    frustrum.m_right);
		auto far_bottom_left   = Geometry::get_intersection(frustrum.m_far,  frustrum.m_bottom, frustrum.m_left);
		auto far_bottom_right  = Geometry::get_intersection(frustrum.m_far,  frustrum.m_bottom, frustrum.m_right);
		ASSERT(near_top_left && near_top_right && near_bottom_left && near_bottom_right && far_top_left && far_top_right && far_bottom_left && far_bottom_right, "Frustrum planes are parallel. Intersection points are required for rendering.");

		{// Draw planes
			OpenGL::DebugRenderer::add(Geometry::Quad(*far_top_left,     *far_top_right,     *far_bottom_left,   *far_bottom_right),  glm::vec4(0.f, 0.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::Quad(*near_top_left,    *near_top_right,    *near_bottom_left,  *near_bottom_right), glm::vec4(0.f, 0.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::Quad(*near_top_left,    *far_top_left,      *near_bottom_left,  *far_bottom_left),   glm::vec4(1.f, 0.f, 0.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::Quad(*near_top_right,   *far_top_right,     *near_bottom_right, *far_bottom_right),  glm::vec4(1.f, 0.f, 0.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::Quad(*near_top_left,    *near_top_right,    *far_top_left,      *far_top_right),     glm::vec4(0.f, 1.f, 0.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::Quad(*near_bottom_left, *near_bottom_right, *far_bottom_left,   *far_bottom_right),  glm::vec4(0.f, 1.f, 0.f, alpha));
		}
		{// Draw corners
			float sphere_radius = glm::distance(*near_top_left, *far_bottom_right) * 0.001f;
			OpenGL::DebugRenderer::add(Geometry::Sphere(*near_top_left,     sphere_radius));
			OpenGL::DebugRenderer::add(Geometry::Sphere(*near_top_right,    sphere_radius));
			OpenGL::DebugRenderer::add(Geometry::Sphere(*near_bottom_left,  sphere_radius));
			OpenGL::DebugRenderer::add(Geometry::Sphere(*near_bottom_right, sphere_radius));
			OpenGL::DebugRenderer::add(Geometry::Sphere(*far_top_left,      sphere_radius));
			OpenGL::DebugRenderer::add(Geometry::Sphere(*far_top_right,     sphere_radius));
			OpenGL::DebugRenderer::add(Geometry::Sphere(*far_bottom_left,   sphere_radius));
			OpenGL::DebugRenderer::add(Geometry::Sphere(*far_bottom_right,  sphere_radius));
		}
		{// Draw connecting line segments
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*near_top_left,     *far_top_left),     glm::vec4(1.f, 1.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*near_top_right,    *far_top_right),    glm::vec4(1.f, 1.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*near_bottom_left,  *far_bottom_left),  glm::vec4(1.f, 1.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*near_bottom_right, *far_bottom_right), glm::vec4(1.f, 1.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*near_top_left,     *near_top_right),   glm::vec4(1.f, 1.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*near_top_right,    *near_bottom_right),glm::vec4(1.f, 1.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*near_bottom_right, *near_bottom_left), glm::vec4(1.f, 1.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*near_bottom_left,  *near_top_left),    glm::vec4(1.f, 1.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*far_top_left,      *far_top_right),    glm::vec4(1.f, 1.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*far_top_right,     *far_bottom_right), glm::vec4(1.f, 1.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*far_bottom_right,  *far_bottom_left),  glm::vec4(1.f, 1.f, 1.f, alpha));
			OpenGL::DebugRenderer::add(Geometry::LineSegment(*far_bottom_left,   *far_top_left),     glm::vec4(1.f, 1.f, 1.f, alpha));
		}
	}
	void DebugRenderer::add(const Geometry::Plane& p_plane, const glm::vec4& p_colour)
	{
		// Because a Plane is infinite, we represent it as a quad bigger than camera z-far which gives it an infinite appearance.
		auto quad = Geometry::Quad(p_plane);
		quad.scale(Z_Far_Scaler);
		add(quad, p_colour);
	}
	void DebugRenderer::add(const Geometry::Point& p_point, const glm::vec4& p_colour)
	{
		add(Geometry::Sphere(p_point.m_position, 0.05f), p_colour);
	}
}