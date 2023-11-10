#pragma once

#include "Shader.hpp"
#include "Utility/MeshBuilder.hpp"

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
		static inline std::optional<Shader> m_debug_shader = std::nullopt;

	public:
		static inline size_t m_quality = 4;

		static void init();
		static void deinit();
		static void clear();
		static void render(System::SceneSystem& p_scene);

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