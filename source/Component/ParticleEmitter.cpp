#include "ParticleEmitter.hpp"
#include "Component/Texture.hpp"

#include "System/TextureSystem.hpp"

#include "imgui.h"

namespace Component
{
	ParticleEmitter::ParticleEmitter(const TextureRef& p_texture)
		: start_texture{p_texture}
		, end_texture{}
		, start_colour{std::nullopt}
		, end_colour{std::nullopt}
		, start_size{1.f}
		, end_size{std::nullopt}
		, emit_position{glm::vec3(0.f)}
		, emit_velocity_min{glm::vec3(0.f, 1.5f, -0.8f)}
		, emit_velocity_max{glm::vec3(0.8f, 2.f,  0.2f)}
		, spawn_period{0.4f}
		, time_to_next_spawn{0.f} // Spawn on creation.
		, lifetime_min{7.f}
		, lifetime_max{7.f}
		, spawn_count{4}
		, max_particle_count{1'000}
		, alive_count{0}
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


			{ImGui::SeparatorText("Render styling");
				ImGui::Text("Colour style", to_string(get_colour_source()));

				{ImGui::SeparatorText("Texture");
					auto textures = get_textures();

					std::vector<std::string> texture_names;
					texture_names.reserve(p_texture_system.m_available_textures.size());
					for (const auto& path : p_texture_system.m_available_textures)
						texture_names.push_back(path.stem().string());

					auto texture_selected_combo = [&](const char* combo_label, TextureRef& current_texture)
					{
						size_t selected_index;
						if (ImGui::ComboContainer(combo_label, current_texture->m_image_ref->name().c_str(), texture_names, selected_index))
							current_texture = p_texture_system.getTexture(p_texture_system.m_available_textures[selected_index]);
					};

					if (textures.first.has_value())
					{
						std::string texture_label = textures.second.has_value() ? "Start texture" : "Texture";
						texture_selected_combo(texture_label.c_str(), textures.first);

						if (start_colour.has_value()) // Only allow removing texture if we are left with a texture source colour.
						{
							ImGui::SameLine();
							if (ImGui::Button("Remove##remove_start_texture_particle_emitter"))
							{
								textures.first  = {};
								textures.second = {};
							}
						}

						if (textures.second.has_value())
						{
							texture_selected_combo("End texture", textures.second);

							ImGui::SameLine();
							if (ImGui::Button("Remove##remove_end_texture_particle_emitter"))
								textures.second = {};
						}
						else
						{
							if (ImGui::Button("Add end texture"))
								textures.second = textures.first;
						}
					}
					else
					{
						if (ImGui::Button("Add##add_texture_particle_emitter"))
							textures.first = p_texture_system.getTexture(p_texture_system.m_available_textures.front());
					}
				}// End texture

				{ImGui::SeparatorText("Colour");
					if (start_colour.has_value())
					{
						std::string colour_edit_label = end_colour.has_value() ? "Start colour" : "Colour";
						ImGui::ColorEdit4(colour_edit_label.c_str(), &start_colour.value().x);

						if (start_texture.has_value()) // Only allow removing colour if we are left with a colour source texture.
						{
							ImGui::SameLine();
							if (ImGui::Button("Remove##remove_start_colour_particle_emitter"))
							{
								start_colour = std::nullopt;
								end_colour   = std::nullopt;
							}
						}

						if (end_colour.has_value())
						{
							ImGui::ColorEdit4("End colour", &end_colour.value().x);
							ImGui::SameLine();
							if (ImGui::Button("Remove##remove_end_colour_particle_emitter"))
								end_colour = std::nullopt;
						}
						else
						{
							if (ImGui::Button("Add end colour"))
								end_colour = start_colour;
						}
					}
					else
					{
						if (ImGui::Button("Add##add_colour_particle_emitter"))
							start_colour = glm::vec4{1.f};
					}
				}// End colour

				{ImGui::SeparatorText("Size");
					std::string size_label = end_size.has_value() ? "Start size" : "Size";
					ImGui::SliderFloat(size_label.c_str(), &start_size, 0.1f, 10.f);

					if (end_size.has_value())
					{
						ImGui::SliderFloat("End size", &end_size.value(), 0.1f, 10.f);
						ImGui::SameLine();
						if (ImGui::Button("Remove##remove_end_size_particle_emitter"))
							end_size = std::nullopt;
					}
					else
					{
						if (ImGui::Button("Add end size"))
							end_size = start_size;
					}
				}// End size
			}

			{ImGui::SeparatorText("Emission");
				ImGui::Slider("Emit position", emit_position, -10.f, 10.f, "%.3fm");

				if (ImGui::Slider("Emit velocity min", emit_velocity_min, -10.f, 10.f, "%.3fm/s"))
				{
					if (emit_velocity_min.x > emit_velocity_max.x) emit_velocity_max.x = emit_velocity_min.x;
					if (emit_velocity_min.y > emit_velocity_max.y) emit_velocity_max.y = emit_velocity_min.y;
					if (emit_velocity_min.z > emit_velocity_max.z) emit_velocity_max.z = emit_velocity_min.z;
				}
				if(ImGui::Slider("Emit velocity max", emit_velocity_max, -10.f, 10.f, "%.3fm/s"))
				{
					if (emit_velocity_max.x < emit_velocity_min.x) emit_velocity_min.x = emit_velocity_max.x;
					if (emit_velocity_max.y < emit_velocity_min.y) emit_velocity_min.y = emit_velocity_max.y;
					if (emit_velocity_max.z < emit_velocity_min.z) emit_velocity_min.z = emit_velocity_max.z;
				}

				ImGui::Slider("Spawn period", spawn_period, DeltaTime(0.f), DeltaTime(10.f), "%.3fs");
				ImGui::Slider("Time to next spawn", time_to_next_spawn, DeltaTime(0.f), DeltaTime(10.f), "%.3fs");
				ImGui::Slider("Spawn count", spawn_count, 0u, 100u);
				ImGui::Slider("Lifetime min", lifetime_min, DeltaTime(0.f), DeltaTime(10.f), "%.3fs");
				ImGui::Slider("Lifetime max", lifetime_max, DeltaTime(0.f), DeltaTime(10.f), "%.3fs");
				ImGui::Slider("Max particle count", max_particle_count, 0u, 1'000'000u);
			}

			{ImGui::SeparatorText("Quick actions");
				if (ImGui::Button("Reset"))
					reset();
			}

			ImGui::TreePop();
		}
	}
	void ParticleEmitter::reset()
	{
		particle_buf.clear();
		alive_count        = 0;
		time_to_next_spawn = spawn_period;
	}
} // namespace Component