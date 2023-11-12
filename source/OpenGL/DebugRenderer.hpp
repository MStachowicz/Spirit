#pragma once

#include "Shader.hpp"
#include "Utility/MeshBuilder.hpp"
#include "Component/Mesh.hpp"

#include "glm/vec4.hpp"
#include <optional>

namespace Geometry
{
	class Cylinder;
	class Frustrum;
	class Plane;
	class Quad;
	class Ray;
	class Sphere;
	class Triangle;
	class LineSegment;
}
namespace System
{
	class SceneSystem;
}

namespace OpenGL
{
	// Immediate mode debug rendering. All members implemented statically to allow ease of use anywhere.
	// Push new debug geometry using add functions.
	// All geometry is cleared at the start of every frame and drawn at the end.
	// DebugRenderer static OpenGL members are all optional to delay their construction till after we have a context and call init().
	// Likewise destruction happens in deinit to allow destructing GL types before we release the context.
	class DebugRenderer
	{
		static inline Utility::MeshBuilder<Data::ColourVertex, PrimitiveMode::Lines> m_line_mb    = {};
		static inline Utility::MeshBuilder<Data::ColourVertex, PrimitiveMode::Triangles> m_tri_mb = {};
		static inline std::optional<Shader> m_debug_shader                                        = {};
		static inline std::optional<Shader> m_bound_shader                                        = {};
		static inline std::optional<Data::Mesh> m_AABB_outline_mesh                               = {};
		static inline std::optional<Data::Mesh> m_AABB_filled_mesh                                = {};
		static inline std::optional<Shader> m_light_position_shader                               = {};
		static inline std::optional<Data::Mesh> m_point_light_mesh                                = {};

	public:
		struct DebugOptions // Options belonging to the Debug Window
		{
			// Rendering
			bool m_show_light_positions  = true;
			float m_light_position_scale = 0.25f;
			bool m_show_mesh_normals     = false;
			// Physics
			bool m_show_orientations                = false; // Draw an arrow in the direction the meshes are facing.
			bool m_show_bounding_box                = false; // Draw the bounding boxes of the meshes. Used for broad phase collision detection.
			bool m_fill_bounding_box                = false; // Fill the bounding boxes of the meshes. Only valid if m_show_bounding_box is true.
			glm::vec4 m_bounding_box_outline_colour = glm::vec4(0.f, 1.f, 0.f, 1.f);
			glm::vec4 m_bounding_box_fill_colour    = glm::vec4(0.f, 1.f, 0.f, 0.2f);
			bool m_show_collision_shape             = false; // Draw the collision shape of the meshes.

			size_t m_quality = 4;
			float m_position_offset_factor = -1.f; // Used to fix z-fighting. Keep this as small as possible.
			float m_position_offset_units  = -1.f; // Used to fix z-fighting. Keep this as small as possible.
		};
		static inline DebugOptions m_debug_options = {};

		static void init();
		static void deinit();
		static void clear();
		static void render(System::SceneSystem& p_scene, const glm::vec3& view_position);

		static void add(const Geometry::Cylinder& p_cylinder, const glm::vec4& p_colour = glm::vec4(1.f));
		static void add(const Geometry::Frustrum& p_frustrum, const glm::vec4& p_colour = glm::vec4(1.f));
		static void add(const Geometry::LineSegment& p_line,  const glm::vec4& p_colour = glm::vec4(1.f));
		static void add(const Geometry::Plane& p_plane,       const glm::vec4& p_colour = glm::vec4(1.f));
		static void add(const Geometry::Quad& p_quad,         const glm::vec4& p_colour = glm::vec4(1.f));
		static void add(const Geometry::Ray& p_ray,           const glm::vec4& p_colour = glm::vec4(1.f));
		static void add(const Geometry::Sphere& p_sphere,     const glm::vec4& p_colour = glm::vec4(1.f));
		static void add(const Geometry::Triangle& p_triangle, const glm::vec4& p_colour = glm::vec4(1.f));
	};
}