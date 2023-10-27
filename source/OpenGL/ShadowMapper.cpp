#include "ShadowMapper.hpp"
#include "OpenGL/DebugRenderer.hpp"

#include "ECS/Storage.hpp"
#include "Component/Camera.hpp"
#include "Component/Lights.hpp"
#include "Component/Transform.hpp"
#include "Component/Mesh.hpp"
#include "System/SceneSystem.hpp"

#include "Geometry/AABB.hpp"
#include "Geometry/Frustrum.hpp"
#include "Geometry/Intersect.hpp"
#include "Geometry/Ray.hpp"

#include "Platform/Window.hpp"

#include "glad/gl.h" // OpenGL functions

#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"

namespace OpenGL
{
    ShadowMapper::ShadowMapper(Platform::Window& p_window) noexcept
        : m_depth_map_FBO{}
        , m_shadow_depth_shader{"shadowDepth"}
        , m_resolution{glm::uvec2(1024 * 4)}
        , m_window{p_window}
    {
        m_depth_map_FBO.attach_depth_buffer(m_resolution);
    }

    void ShadowMapper::shadow_pass(System::Scene& p_scene)
    {
        ASSERT(m_depth_map_FBO.isComplete(), "[OPENGL][SHADOW MAPPER] framebuffer not complete, have you attached a depth buffer + empty draw and read buffers.");

        unsigned int directional_light_count = 0;
        p_scene.m_entities.foreach([&directional_light_count](Component::DirectionalLight& p_light) { directional_light_count++; });

        if (directional_light_count > 0)
        {
            m_depth_map_FBO.bind();
            set_viewport(0, 0, m_resolution.x, m_resolution.y);
            m_depth_map_FBO.clearBuffers();
            m_shadow_depth_shader.use();
            set_cull_face(false);

            set_depth_test(true);
            set_depth_test_type(DepthTestType::Less);
            set_polygon_mode(PolygonMode::Fill);

            // Draw the scene from the perspective of the light
            p_scene.m_entities.foreach([&m_shadow_depth_shader = this->m_shadow_depth_shader, &p_scene](Component::DirectionalLight& p_light)
            {
                {// Create the light view projection matrix based on the bounds of the scene.
                    m_shadow_depth_shader.set_uniform("light_space_matrix", p_light.get_view_proj(p_scene.m_bound));
                }

                p_scene.m_entities.foreach([&m_shadow_depth_shader](Component::Transform& p_transform, Component::Mesh& p_mesh)
                {
                    m_shadow_depth_shader.set_uniform("model", p_transform.mModel);
                    p_mesh.mModel->forEachMesh([](const Data::Mesh& p_mesh_data)
                    {
                        p_mesh_data.mGLData.draw();
                    });
                });
            });
            m_depth_map_FBO.unbind();
        }
    }

    void ShadowMapper::draw_UI()
    {

    }
} // namespace OpenGL