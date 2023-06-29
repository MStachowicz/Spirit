#include "Types.hpp"
#include "Shader.hpp"

namespace ECS
{
    class Storage;
}
namespace OpenGL
{
    class LightPositionRenderer
    {
        Shader m_light_position_shader;
        float m_light_position_scale;

    public:
        LightPositionRenderer() noexcept;

        void draw(ECS::Storage& p_storage, const Mesh& p_light_mesh);
    };
}