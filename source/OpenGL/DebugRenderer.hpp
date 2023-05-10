#pragma once

#include "Types.hpp"
#include "Shader.hpp"

#include "glm/vec3.hpp"

#include <array>
#include <optional>
#include <vector>

namespace Geometry
{
    class Cylinder;
    class Plane;
    class Ray;
    class Sphere;
    class Triangle;
}
namespace ECS
{
    class Storage;
}
namespace System
{
    class SceneSystem;
}

namespace OpenGL
{
    struct DebugVert
    {
        constexpr inline static std::array<VertexAttribute, 2> Attributes =
        {
            VertexAttribute::Position3D,
            VertexAttribute::ColourRGB
        };

        glm::vec3 position;
        glm::vec3 colour;
    };

    // Immediate mode debug rendering. All members implemented statically to allow ease of use anywhere.
    // Push new debug geometry using add functions.
    // All geometry is cleared at the start of every frame and drawn at the end.
    // DebugRenderer static OpenGL members are all optional to delay their construction till after we have a context and call init().
    // Likewise destruction happens in deinit to allow destructing GL types before we release the context.
    class DebugRenderer
    {

        static inline std::vector<DebugVert> m_debug_verts = {};
        static inline std::optional<VAO> m_debug_VAO = std::nullopt;
        static inline std::optional<VBO> m_debug_VBO = std::nullopt;
        static inline std::optional<Shader> m_debug_shader = std::nullopt;

    public:
        static void init();
        static void deinit();

        static void clear();
        static void render(System::SceneSystem& p_scene);

        static void add(const Geometry::Cylinder& p_cylinder, const glm::vec3& p_colour = glm::vec3(1.f));
        static void add(const Geometry::Plane& p_plane, const glm::vec3& p_colour = glm::vec3(1.f));
        static void add(const Geometry::Ray& p_ray, const glm::vec3& p_colour = glm::vec3(1.f));
        static void add(const Geometry::Sphere& p_sphere, const glm::vec3& p_colour = glm::vec3(1.f));
        static void add(const Geometry::Triangle& p_triangle, const glm::vec3& p_colour = glm::vec3(1.f));
    };
}