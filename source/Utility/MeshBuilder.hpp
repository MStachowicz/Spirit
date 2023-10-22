#pragma once

#include "Logger.hpp"

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include "OpenGL/Types.hpp"
#include "OpenGL/GLState.hpp"

#include "glm/glm.hpp"

#include <vector>

namespace Utility
{
    class Vertex
    {
    public:
        glm::vec3 position = glm::vec3{0.f};
        glm::vec3 normal   = glm::vec3{0.f};
        glm::vec2 uv       = glm::vec2{0.f};
        glm::vec3 colour   = glm::vec3{1.f};
    };

    class Mesh
    {
        OpenGL::VAO VAO   = {};
        OpenGL::VBO VBO   = {};
        GLsizei draw_size = 0;
        OpenGL::PrimitiveMode primitive_mode;

    public:
        void draw()
        {
            VAO.bind();
            OpenGL::draw_arrays(primitive_mode, 0, draw_size);
        }

        Mesh(const std::vector<Vertex>& vertex_data, OpenGL::PrimitiveMode primitive_mode)
            : VAO{}
            , VBO{}
            , draw_size{(GLsizei)vertex_data.size()}
            , primitive_mode{primitive_mode}
        {
            VAO.bind();
            VBO.bind();

            OpenGL::buffer_data(OpenGL::BufferType::ArrayBuffer, vertex_data.size() * sizeof(Vertex), vertex_data.data(), OpenGL::BufferUsage::StaticDraw);

            OpenGL::vertex_attrib_pointer(0, 3, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
            OpenGL::vertex_attrib_pointer(1, 3, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            OpenGL::vertex_attrib_pointer(3, 2, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, uv));
            OpenGL::vertex_attrib_pointer(2, 3, OpenGL::ShaderDataType::Float, false, sizeof(Vertex), (void*)offsetof(Vertex, colour));

            OpenGL::enable_vertex_attrib_array(0);
            OpenGL::enable_vertex_attrib_array(1);
            OpenGL::enable_vertex_attrib_array(2);
            OpenGL::enable_vertex_attrib_array(3);
        }
    };

    class MeshBuilder
    {
    public:
        MeshBuilder(OpenGL::PrimitiveMode mode) noexcept
            : data{}
            , current_colour{glm::vec3{1.f}}
            , primitive_mode{mode}
        {}

        void add_vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2 uv = {0,0})
        {
            if (primitive_mode == OpenGL::PrimitiveMode::Points)
                data.emplace_back(Vertex{position, normal, uv, current_colour});
            else
                ASSERT(false, "add_vertex for this primitive mode is not supported.");
        }

        void add_triangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec2 uv1 = {0,0}, const glm::vec2 uv2 = {0,0}, const glm::vec2 uv3 = {0,0})
        {
            if (primitive_mode == OpenGL::PrimitiveMode::Triangles)
            {
                const auto v1     = p2 - p1;
                const auto v2     = p3 - p1;
                const auto normal = glm::cross(v1, v2);

                data.emplace_back(Vertex{p1, normal, uv1, current_colour});
                data.emplace_back(Vertex{p2, normal, uv2, current_colour});
                data.emplace_back(Vertex{p3, normal, uv3, current_colour});
            }
            else
                ASSERT(false, "add_triangle for this primitive mode is not supported.");

        }
        void add_quad(const glm::vec3& top_left, const glm::vec3& top_right, const glm::vec3& bottom_left, const glm::vec3& bottom_right)
        {
            if (primitive_mode == OpenGL::PrimitiveMode::Triangles)
            {// Reverse winding order to ensure the normal is facing the correct way.
                add_triangle(top_left, bottom_left, bottom_right,     glm::vec2(0.f, 1.f), glm::vec2(0.f, 0.f), glm::vec2(1.f , 0.f));
                add_triangle(top_left, bottom_right, top_right,       glm::vec2(0.f, 1.f), glm::vec2(1.f, 0.f), glm::vec2(1.f , 1.f));
            }
            else
                ASSERT(false, "add_quad for this primitive mode is not supported.");
        }
        void resereve(size_t size)
        {
            data.reserve(size);
        }
        void set_colour(const glm::vec3& colour)
        {
            current_colour = colour;
        }
        [[NODISCARD]] Mesh get_mesh()
        {
            return Mesh{data, primitive_mode};
        }

    private:
        std::vector<Vertex> data;
        glm::vec3 current_colour;
        const OpenGL::PrimitiveMode primitive_mode;
    };
}