#include "Terrain.hpp"

#include "System/TextureSystem.hpp"

#include "Utility/MeshBuilder.hpp"
#include "Utility/PerlinNoise.hpp"

#include <imgui.h>

Component::Terrain::Terrain(int size_x, int size_z) noexcept
    : position{glm::vec3(0.f)}
    , size_x{size_x}
    , size_z{size_z}
    , scale_factor{1.f}
    , texture{}
    , mesh{generate_mesh()}
{}

float compute_height(float x, float z, float scale_factor, const siv::PerlinNoise& perlin)
{
    return perlin.noise2D(x * scale_factor, z * scale_factor);
}

Data::NewMesh Component::Terrain::generate_mesh() noexcept
{
    // Use perlin noise to generate a heightmap in the xz plane.
    auto mb = Utility::MeshBuilder<Data::Vertex, OpenGL::PrimitiveMode::Triangles>{};
    const siv::PerlinNoise::seed_type seed = 123456u;
    const siv::PerlinNoise perlin{seed};

    for (int x = 0; x < size_x; x++)
        for (int z = 0; z < size_z; z++)
        {
            mb.add_quad(
                glm::vec3(x + 1, compute_height(x + 1, z,     scale_factor, perlin), z),
                glm::vec3(x + 1, compute_height(x + 1, z + 1, scale_factor, perlin), z + 1),
                glm::vec3(x,     compute_height(x, z,         scale_factor, perlin), z),
                glm::vec3(x,     compute_height(x, z + 1,     scale_factor, perlin), z  + 1));
        }

    return mb.get_mesh();
}

void Component::Terrain::draw_UI(System::TextureSystem& texture_system)
{
    if (ImGui::TreeNode("Terrain"))
    {
        {// Texture settings
            std::vector<std::string> texture_names;
            texture_names.reserve(texture_system.mAvailableTextures.size());
            for (const auto& path : texture_system.mAvailableTextures)
                texture_names.push_back(path.stem().string());

            size_t selected_index;
            std::string current = texture ? texture->m_image_ref->name() : "None";
            if (ImGui::ComboContainer("Texture", current.c_str(), texture_names, selected_index))
                texture = texture_system.getTexture(texture_system.mAvailableTextures[selected_index]);
        }

        ImGui::Slider("Position", position, -100.f, 100.f, "%.3fm");
        ImGui::Slider("Size X", size_x, 1, 1000, "%dm");
        ImGui::Slider("Size Z", size_z, 1, 1000, "%dm");
        ImGui::Slider("Scale factor", scale_factor, 0.01f , 10.f);

        if (ImGui::Button("Re-generate terrain"))
            mesh = generate_mesh();

        ImGui::TreePop();
    }
}