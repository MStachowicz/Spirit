#include "ParticleEmitter.hpp"
#include "Component/Texture.hpp"

#include "System/TextureSystem.hpp"

#include "imgui.h"

namespace Component
{
	ParticleEmitter::ParticleEmitter(const TextureRef& p_texture)
		: diffuse{p_texture}
		, emit_position{glm::vec3(0.f)}
		, emit_velocity_min{glm::vec3(0.f, 1.5f, -0.8f)}
		, emit_velocity_max{glm::vec3(0.8f, 2.f,  0.2f)}
		, spawn_period{0.4f}
		, time_to_next_spawn{0.f} // Spawn on creation.
		, lifetime{7.f}
		, spawn_count{4}
		, max_particle_count{1'000}
		, alive_count{0}
		, sort_by_distance_to_camera{false}
		, particle_buf{OpenGL::BufferStorageBitfield({OpenGL::BufferStorageFlag::DynamicStorageBit})}
	{
		ASSERT_THROW(emit_velocity_min.x < emit_velocity_max.x
			&& emit_velocity_min.y < emit_velocity_max.y
			&& emit_velocity_min.z < emit_velocity_max.z, "ParticleEmitter min not smaller than max");
	}

	void ParticleEmitter::draw_UI(System::TextureSystem& p_texture_system)
	{
		if (ImGui::TreeNode("Paticle Emitter"))
		{
			ImGui::Text("Particle count", alive_count);
			ImGui::Text("Particlebuffer size", particle_buf.size());

			std::vector<std::string> texture_names;
			texture_names.reserve(p_texture_system.m_available_textures.size());
			for (const auto& path : p_texture_system.m_available_textures)
				texture_names.push_back(path.stem().string());

			size_t selected_index;
			if (ImGui::ComboContainer("Texture", diffuse->m_image_ref->name().c_str(), texture_names, selected_index))
				diffuse = p_texture_system.getTexture(p_texture_system.m_available_textures[selected_index]);

			ImGui::Slider("Emit position", emit_position, -10.f, 10.f, "%.3fm");

			constexpr auto correction_magnitude = 1.f;
			if (ImGui::Slider("Emit velocity min", emit_velocity_min, -10.f, 10.f, "%.3fm/s"))

			if (emit_velocity_min.x > emit_velocity_max.x)
				emit_velocity_max.x = emit_velocity_min.x + correction_magnitude;
			if (emit_velocity_min.y > emit_velocity_max.y)
				emit_velocity_max.y = emit_velocity_min.y + correction_magnitude;
			if (emit_velocity_min.z > emit_velocity_max.z)
				emit_velocity_max.z = emit_velocity_min.z + correction_magnitude;

			if (ImGui::Slider("Emit velocity max", emit_velocity_max, -10.f, 10.f, "%.3fm/s"))

			if (emit_velocity_max.x < emit_velocity_min.x)
				emit_velocity_min.x = emit_velocity_max.x - correction_magnitude;
			if (emit_velocity_max.y < emit_velocity_min.y)
				emit_velocity_min.y = emit_velocity_max.y - correction_magnitude;
			if (emit_velocity_max.z < emit_velocity_min.z)
				emit_velocity_min.z = emit_velocity_max.z - correction_magnitude;

			ASSERT_THROW(emit_velocity_min.x < emit_velocity_max.x
				&& emit_velocity_min.y < emit_velocity_max.y
				&& emit_velocity_min.z < emit_velocity_max.z, "ParticleEmitter min not smaller than max");

			ImGui::Slider("Spawn period", spawn_period, DeltaTime(0.f), DeltaTime(10.f), "%.3fs");
			ImGui::Slider("Time to next spawn", time_to_next_spawn, DeltaTime(0.f), DeltaTime(10.f), "%.3fs");
			ImGui::Slider("Spawn count", spawn_count, 0u, 100u);
			ImGui::Slider("Lifetime", lifetime, DeltaTime(0.f), DeltaTime(10.f), "%.3fs");
			ImGui::Slider("Max particle count", max_particle_count, 0u, 1'000'000u);
			ImGui::Checkbox("Sort by distance to camera", &sort_by_distance_to_camera);

			ImGui::TreePop();
		}
	}
}