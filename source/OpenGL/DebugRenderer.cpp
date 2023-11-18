#include "DebugRenderer.hpp"
#include "OpenGL/GLState.hpp"
#include "OpenGL/DrawCall.hpp"

#include "Component/Collider.hpp"
#include "Component/Lights.hpp"
#include "Component/Transform.hpp"
#include "ECS/Storage.hpp"
#include "System/SceneSystem.hpp"

#include "Geometry/Cylinder.hpp"
#include "Geometry/Frustrum.hpp"
#include "Geometry/Geometry.hpp"
#include "Geometry/Intersect.hpp"
#include "Geometry/LineSegment.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Quad.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Sphere.hpp"
#include "Geometry/Triangle.hpp"

namespace OpenGL
{
	static constexpr float Z_Far_Scaler = 1000.f; // Scale the geometry that extends infinitely by this to give it an infinite appearance.

	void DebugRenderer::init()
	{
		m_debug_shader          = {"DebugRender"};
		m_bound_shader          = {"uniformColour"};
		m_light_position_shader = {"light_position"};

		{// Create a cube meshes to represent AABBs.
			{
				auto mb = Utility::MeshBuilder<Data::PositionVertex, PrimitiveMode::Lines>{};
				mb.add_cuboid(glm::vec3(0.f), glm::vec3(1.f));
				m_AABB_outline_mesh = mb.get_mesh();
			}
			{
				auto mb = Utility::MeshBuilder<Data::PositionVertex, PrimitiveMode::Triangles>{};
				mb.add_cuboid(glm::vec3(0.f), glm::vec3(1.f));
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
	void DebugRenderer::render(System::SceneSystem& p_scene, const glm::vec3& view_position)
	{
		auto line_mesh = m_line_mb.get_mesh();
		if (!line_mesh.empty())
		{
			DrawCall dc;
			dc.m_cull_face_enabled = false;
			dc.submit(m_debug_shader.value(), line_mesh);
		}
		auto tri_mesh = m_tri_mb.get_mesh();
		if (!tri_mesh.empty())
		{
			DrawCall dc;
			dc.m_cull_face_enabled = false;
			dc.submit(m_debug_shader.value(), tri_mesh);
		}

		auto& scene = p_scene.getCurrentScene();
		auto& opt = m_debug_options;

		if (opt.m_show_bounding_box)
		{
			scene.foreach([&](Component::Transform& p_transform, Component::Mesh& p_mesh, Component::Collider& p_collider)
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
					dc.submit(*m_bound_shader, *m_AABB_outline_mesh);
				}
				if (opt.m_fill_bounding_box)
				{
					DrawCall dc;
					dc.m_cull_face_enabled      = false;
					dc.m_polygon_offset_enabled = true;
					dc.m_polygon_offset_factor  = opt.m_position_offset_factor;
					dc.m_polygon_offset_units   = opt.m_position_offset_units;
					dc.set_uniform("model", model);
					dc.set_uniform("colour", p_collider.m_collided ? glm::vec4(opt.m_bounding_box_collided_colour, 0.2f) : glm::vec4(opt.m_bounding_box_colour, 0.2f));
					dc.submit(*m_bound_shader, *m_AABB_filled_mesh);
				}
			});
		}
		if (opt.m_show_light_positions)
		{
			int point_light_count = 0;
			scene.foreach([&point_light_count](Component::PointLight& point_light) { point_light_count++; });

			if (point_light_count > 0)
			{
				DrawCall dc;
				dc.set_uniform("scale", opt.m_light_position_scale);
				dc.submit(*m_light_position_shader, *m_point_light_mesh, point_light_count);
			}
		}
	}

	void DebugRenderer::add(const Geometry::Triangle& p_triangle, const glm::vec4& p_colour)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_triangle(p_triangle.m_point_1, p_triangle.m_point_2, p_triangle.m_point_3);
	}
	void DebugRenderer::add(const Geometry::Cylinder& p_cylinder, const glm::vec4& p_colour)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_cylinder(p_cylinder.m_base, p_cylinder.m_top, p_cylinder.m_radius, m_debug_options.m_quality);
	}
	void DebugRenderer::add(const Geometry::Quad& p_quad, const glm::vec4& p_colour)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_quad(p_quad.m_point_1, p_quad.m_point_2, p_quad.m_point_3, p_quad.m_point_4);
	}
	void DebugRenderer::add(const Geometry::LineSegment& p_line, const glm::vec4& p_colour)
	{
		m_line_mb.set_colour(p_colour);
		m_line_mb.add_line(p_line.m_start, p_line.m_end);
	}
	void DebugRenderer::add(const Geometry::Ray& p_ray, const glm::vec4& p_colour)
	{
		// Because a Ray extends infinitely, we represent it as a Line extending beyond camera z-far which gives it an infinite appearance.
		m_line_mb.set_colour(p_colour);
		m_line_mb.add_line(p_ray.m_start, p_ray.m_start + (p_ray.m_direction * Z_Far_Scaler));
	}
	void DebugRenderer::add(const Geometry::Sphere& p_sphere, const glm::vec4& p_colour)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_icosphere(p_sphere.m_center, p_sphere.m_radius, m_debug_options.m_quality);
	}
	void DebugRenderer::add(const Geometry::Frustrum& p_frustrum, const glm::vec4& p_colour)
	{
		if (auto near_top_left     = Geometry::get_intersection(p_frustrum.m_near, p_frustrum.m_top,    p_frustrum.m_left))
        if (auto near_top_right    = Geometry::get_intersection(p_frustrum.m_near, p_frustrum.m_top,    p_frustrum.m_right))
        if (auto near_bottom_left  = Geometry::get_intersection(p_frustrum.m_near, p_frustrum.m_bottom, p_frustrum.m_left))
        if (auto near_bottom_right = Geometry::get_intersection(p_frustrum.m_near, p_frustrum.m_bottom, p_frustrum.m_right))
        if (auto far_top_left      = Geometry::get_intersection(p_frustrum.m_far,  p_frustrum.m_top,    p_frustrum.m_left))
        if (auto far_top_right     = Geometry::get_intersection(p_frustrum.m_far,  p_frustrum.m_top,    p_frustrum.m_right))
        if (auto far_bottom_left   = Geometry::get_intersection(p_frustrum.m_far,  p_frustrum.m_bottom, p_frustrum.m_left))
        if (auto far_bottom_right  = Geometry::get_intersection(p_frustrum.m_far,  p_frustrum.m_bottom, p_frustrum.m_right))
		{
			add(Geometry::Sphere(*near_top_left, 0.1f));
			add(Geometry::Sphere(*near_top_right, 0.1f));
			add(Geometry::Sphere(*near_bottom_left, 0.1f));
			add(Geometry::Sphere(*near_bottom_right, 0.1f));
			add(Geometry::Sphere(*far_top_left, 0.1f));
			add(Geometry::Sphere(*far_top_right, 0.1f));
			add(Geometry::Sphere(*far_bottom_left, 0.1f));
			add(Geometry::Sphere(*far_bottom_right, 0.1f));

			add(Geometry::Quad(*near_top_left,    *far_top_left,      *far_bottom_left,   *near_bottom_left),  glm::vec4(1.f, 0.f, 0.f, p_colour.w));
			add(Geometry::Quad(*near_top_right,   *far_top_right,     *far_bottom_right,  *near_bottom_right), glm::vec4(1.f, 0.f, 0.f, p_colour.w));
			add(Geometry::Quad(*near_top_left,    *near_top_right,    *far_top_right,     *far_top_left),      glm::vec4(0.f, 1.f, 0.f, p_colour.w));
			add(Geometry::Quad(*near_bottom_left, *near_bottom_right, *far_bottom_right,  *far_bottom_left),   glm::vec4(0.f, 1.f, 0.f, p_colour.w));
			add(Geometry::Quad(*near_top_left,    *near_top_right,    *near_bottom_right, *near_bottom_left),  glm::vec4(0.f, 0.f, 1.f, p_colour.w));
			add(Geometry::Quad(*far_top_left,     *far_top_right,     *far_bottom_right,  *far_bottom_left),   glm::vec4(0.f, 0.f, 1.f, p_colour.w));
			return;
		}

		{
			add(p_frustrum.m_left,   p_colour);
			add(p_frustrum.m_right,  p_colour);
			add(p_frustrum.m_bottom, p_colour);
			add(p_frustrum.m_top,    p_colour);
			add(p_frustrum.m_near,   p_colour);
			add(p_frustrum.m_far,    p_colour);
		}
	}
	void DebugRenderer::add(const Geometry::Plane& p_plane, const glm::vec4& p_colour)
	{
		// Because a Plane is infinite, we represent it as a quad bigger than camera z-far which gives it an infinite appearance.
		auto quad = Geometry::Quad(p_plane);
		quad.scale(Z_Far_Scaler);
		add(quad, p_colour);
	}
}