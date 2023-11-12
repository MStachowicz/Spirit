#include "DebugRenderer.hpp"
#include "OpenGL/GLState.hpp"
#include "OpenGL/DrawCall.hpp"

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
		m_debug_shader = {"DebugRender"};
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
	void DebugRenderer::render(System::SceneSystem& p_scene)
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
	}

	void DebugRenderer::add(const Geometry::Triangle& p_triangle, const glm::vec4& p_colour)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_triangle(p_triangle.m_point_1, p_triangle.m_point_2, p_triangle.m_point_3);
	}
	void DebugRenderer::add(const Geometry::Cylinder& p_cylinder, const glm::vec4& p_colour)
	{
		m_tri_mb.set_colour(p_colour);
		m_tri_mb.add_cylinder(p_cylinder.m_base, p_cylinder.m_top, p_cylinder.m_radius, m_quality);
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
		m_tri_mb.add_icosphere(p_sphere.m_center, p_sphere.m_radius, m_quality);
	}
	void DebugRenderer::add(const Geometry::Frustrum& p_frustrum, const glm::vec4& p_colour)
	{
		glm::vec3 near_top_left, near_top_right, near_bottom_left, near_bottom_right, far_top_left, far_top_right, far_bottom_left, far_bottom_right;

		if (Geometry::intersect_plane_plane_plane(p_frustrum.m_near,    p_frustrum.m_top,    p_frustrum.m_left,  near_top_left)
			&& Geometry::intersect_plane_plane_plane(p_frustrum.m_near, p_frustrum.m_top,    p_frustrum.m_right, near_top_right)
			&& Geometry::intersect_plane_plane_plane(p_frustrum.m_near, p_frustrum.m_bottom, p_frustrum.m_left,  near_bottom_left)
			&& Geometry::intersect_plane_plane_plane(p_frustrum.m_near, p_frustrum.m_bottom, p_frustrum.m_right, near_bottom_right)
			&& Geometry::intersect_plane_plane_plane(p_frustrum.m_far,  p_frustrum.m_top,    p_frustrum.m_left,  far_top_left)
			&& Geometry::intersect_plane_plane_plane(p_frustrum.m_far,  p_frustrum.m_top,    p_frustrum.m_right, far_top_right)
			&& Geometry::intersect_plane_plane_plane(p_frustrum.m_far,  p_frustrum.m_bottom, p_frustrum.m_left,  far_bottom_left)
			&& Geometry::intersect_plane_plane_plane(p_frustrum.m_far,  p_frustrum.m_bottom, p_frustrum.m_right, far_bottom_right))
		{ // If possible, collide all the planes to find the quad positions representing the frustrum.
			add(Geometry::Sphere(near_top_left, 0.1f));
			add(Geometry::Sphere(near_top_right, 0.1f));
			add(Geometry::Sphere(near_bottom_left, 0.1f));
			add(Geometry::Sphere(near_bottom_right, 0.1f));
			add(Geometry::Sphere(far_top_left, 0.1f));
			add(Geometry::Sphere(far_top_right, 0.1f));
			add(Geometry::Sphere(far_bottom_left, 0.1f));
			add(Geometry::Sphere(far_bottom_right, 0.1f));

			add(Geometry::Quad(near_top_left, far_top_left, far_bottom_left, near_bottom_left),         glm::vec4(1.f, 0.f, 0.f, p_colour.w));
			add(Geometry::Quad(near_top_right, far_top_right, far_bottom_right, near_bottom_right),     glm::vec4(1.f, 0.f, 0.f, p_colour.w));
			add(Geometry::Quad(near_top_left, near_top_right, far_top_right, far_top_left),             glm::vec4(0.f, 1.f, 0.f, p_colour.w));
			add(Geometry::Quad(near_bottom_left, near_bottom_right, far_bottom_right, far_bottom_left), glm::vec4(0.f, 1.f, 0.f, p_colour.w));
			add(Geometry::Quad(near_top_left, near_top_right, near_bottom_right, near_bottom_left),     glm::vec4(0.f, 0.f, 1.f, p_colour.w));
			add(Geometry::Quad(far_top_left, far_top_right, far_bottom_right, far_bottom_left),         glm::vec4(0.f, 0.f, 1.f, p_colour.w));
		}
		else
		{
			add(p_frustrum.m_left, p_colour);
			add(p_frustrum.m_right, p_colour);
			add(p_frustrum.m_bottom, p_colour);
			add(p_frustrum.m_top, p_colour);
			add(p_frustrum.m_near, p_colour);
			add(p_frustrum.m_far, p_colour);
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