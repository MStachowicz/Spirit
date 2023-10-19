#include "ParticleEmitter.hpp"
#include "Component/Texture.hpp"

#include "System/TextureSystem.hpp"

#include "External/ImGui/imgui.h"

namespace Component
{
    ParticleEmitter::ParticleEmitter(const TextureRef& p_texture)
        : diffuse{p_texture}
        , emit_position{glm::vec3(0.f)}
        , emit_velocity_min{glm::vec3(-0.5f, 0.9f, -0.5f)}
        , emit_velocity_max{glm::vec3( 0.5f, 1.f,  0.5f)}
        , spawn_period{1.f}
        , time_to_next_spawn{0.f} // Spawn on creation.
        , spawn_count{3u}
        , lifetime{7.f}
        , max_particle_count{1'000u}
        , particles{}
    {
        ASSERT(emit_velocity_min.x < emit_velocity_max.x
            && emit_velocity_min.y < emit_velocity_max.y
            && emit_velocity_min.z < emit_velocity_max.z, "ParticleEmitter min not smaller than max"); // Always

        particles.reserve(max_particle_count);
    }

    void ParticleEmitter::draw_UI(System::TextureSystem& p_texture_system)
    {
        if (ImGui::TreeNode("Paticle Emitter"))
        {
            ImGui::Text("Particle count: %zu", particles.size());

            std::vector<std::string> texture_names;
            texture_names.reserve(p_texture_system.mAvailableTextures.size());
            for (const auto& path : p_texture_system.mAvailableTextures)
                texture_names.push_back(path.stem().string());

            size_t selected_index;
            if (ImGui::ComboContainer("Texture", diffuse->m_image_ref->name().c_str(), texture_names, selected_index))
                diffuse = p_texture_system.getTexture(p_texture_system.mAvailableTextures[selected_index]);

            ImGui::Slider("Emit position", emit_position, -10.f, 10.f, "%.3fm");

            constexpr auto correction_magnitude = 1.f;
            if (ImGui::Slider("Emit velocity min", emit_velocity_min, -10.f, 10.f, "%.3fm/s"))
            {
                if (emit_velocity_min.x > emit_velocity_max.x)
                    emit_velocity_max.x = emit_velocity_min.x + correction_magnitude;
                if (emit_velocity_min.y > emit_velocity_max.y)
                    emit_velocity_max.y = emit_velocity_min.y + correction_magnitude;
                if (emit_velocity_min.z > emit_velocity_max.z)
                    emit_velocity_max.z = emit_velocity_min.z + correction_magnitude;
            }
            if (ImGui::Slider("Emit velocity max", emit_velocity_max, -10.f, 10.f, "%.3fm/s"))
            {
                if (emit_velocity_max.x < emit_velocity_min.x)
                    emit_velocity_min.x = emit_velocity_max.x - correction_magnitude;
                if (emit_velocity_max.y < emit_velocity_min.y)
                    emit_velocity_min.y = emit_velocity_max.y - correction_magnitude;
                if (emit_velocity_max.z < emit_velocity_min.z)
                    emit_velocity_min.z = emit_velocity_max.z - correction_magnitude;
            }

            ASSERT(emit_velocity_min.x < emit_velocity_max.x
                && emit_velocity_min.y < emit_velocity_max.y
                && emit_velocity_min.z < emit_velocity_max.z, "ParticleEmitter min not smaller than max"); // Always

            ImGui::Slider("Spawn period", spawn_period, DeltaTime(0.f), DeltaTime(10.f), "%.3fs");
            ImGui::Slider("Time to next spawn", time_to_next_spawn, DeltaTime(0.f), DeltaTime(10.f), "%.3fs");
            ImGui::Slider("Spawn count", spawn_count, 0u, 100u);
            ImGui::Slider("Lifetime", lifetime, DeltaTime(0.f), DeltaTime(10.f), "%.3fs");
            ImGui::Slider("Max particle count", max_particle_count, 0u, 1'000'000u);
            ImGui::TreePop();
        }
    }
}