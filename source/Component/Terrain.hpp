#include "MeshBuilder.hpp"

#include "Component/Texture.hpp"

namespace System
{
    class TextureSystem;
}
namespace Component
{
    class Terrain
    {
        Utility::Mesh generate_mesh() noexcept;

    public:
        glm::vec3 position;
        int size_x;
        int size_z;
        float scale_factor;
        TextureRef texture;
        Utility::Mesh mesh;

        Terrain(int size_x, int size_z) noexcept;
        void draw_UI(System::TextureSystem& texture_system);
    };
} // namespace Component