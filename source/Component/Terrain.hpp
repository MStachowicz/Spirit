#include "Component/Texture.hpp"
#include "Component/Mesh.hpp"

namespace System
{
    class TextureSystem;
}
namespace Component
{
    class Terrain
    {
        Data::NewMesh generate_mesh() noexcept;

    public:
        glm::vec3 position;
        int size_x;
        int size_z;
        float scale_factor;
        TextureRef texture;
        Data::NewMesh mesh;

        Terrain(int size_x, int size_z) noexcept;
        void draw_UI(System::TextureSystem& texture_system);
    };
} // namespace Component