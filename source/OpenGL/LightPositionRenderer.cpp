#include "LightPositionRenderer.hpp"
#include "GLState.hpp"

#include "Component/Lights.hpp"

#include "ECS/Storage.hpp"

namespace OpenGL
{
        LightPositionRenderer::LightPositionRenderer() noexcept
            : m_light_position_shader{"light_position"}
            , m_light_position_scale{0.25f}
        {}

        void LightPositionRenderer::draw(ECS::Storage& p_storage, const Mesh& p_light_mesh)
        {
            int point_light_count = 0;
            p_storage.foreach ([&point_light_count](Component::PointLight& point_light) { point_light_count++; });

            if (point_light_count > 0)
            {
                m_light_position_shader.use();
                m_light_position_shader.set_uniform("scale", m_light_position_scale);

                p_light_mesh.mVAO.bind();
                if (p_light_mesh.mEBO.has_value())
                    draw_elements_instanced(PrimitiveMode::Triangles, p_light_mesh.mDrawSize, point_light_count);
                else
                    draw_arrays_instanced(PrimitiveMode::Triangles, p_light_mesh.mDrawSize, point_light_count);
            }
        }
}