#pragma once

#include "Component/Texture.hpp"
#include "Utility/Config.hpp"

#include "OpenGL/Types.hpp"

#include "glm/vec4.hpp"

namespace System
{
	class TextureSystem;
}
namespace Component
{
	struct Particle // Matches the Particle struct in particle shaders.
	{
		glm::vec4 position;
		glm::vec4 velocity;
	};

	class ParticleEmitter
	{
	public:
		constexpr static size_t Persistent_ID = 8;

		// start_texture and end_texture are optional.
		// start_texture is required for end_texture to be available.
		TextureRef start_texture; // Texture to use at the start of the particle's life. If end_texture is not set, this texture is used for the entire life of the particle.
		TextureRef end_texture;   // Texture to use at the end of the particle's life. If not set, the start_texture is used for the entire life of the particle.

		// start_colour and end_colour are optional.
		// start_colour is required for end_colour to be available.
		std::optional<glm::vec4> start_colour; // Colour to use at the start of the particle's life. If end_colour is not set, this colour is used for the entire life of the particle.
		std::optional<glm::vec4> end_colour; // Colour to use at the end of the particle's life. If not set, the start_colour is used for the entire life of the particle.

		// start_size is required, end_size is optional.
		// If end_size is set, the size of the particle will interpolate between start_size and end_size over its lifetime.
		float start_size;
		std::optional<float> end_size;


		enum class ColourSource : uint8_t
		{
			ConstantColour,
			ConstantTexture,
			ConstantColourAndTexture,
			VaryingColour,
			VaryingTexture,
			VaryingColourConstantTexture,
			ConstantColourVaryingTexture,
			VaryingColourAndTexture
		};
		ColourSource get_colour_source() const
		{
			if (start_texture.has_value()) // Textured
			{
				if (end_texture.has_value()) // Varying texture
				{
					if (start_colour.has_value()) // Coloured
					{
						if (end_colour.has_value()) // Varying colour
							return ColourSource::VaryingColourAndTexture;
						else
							return ColourSource::ConstantColourVaryingTexture;
					}
					else
						return ColourSource::VaryingTexture;
				}
				else // Constant texture.
				{
					if (start_colour.has_value()) // Coloured
					{
						if (end_colour.has_value()) // Varying colour
							return ColourSource::VaryingColourConstantTexture;
						else
							return ColourSource::ConstantColourAndTexture;
					}
					else
						return ColourSource::ConstantTexture;
				}
			}
			else if (start_colour.has_value()) // No texture, coloured.
			{
				if (end_colour.has_value()) // Varying colour.
					return ColourSource::VaryingColour;
				else
					return ColourSource::ConstantColour;
			}
			else // Invalid state
				ASSERT_THROW(false, "Invalid colour source state. ParticleEmitter needs either a colour or a texture.")
		}
		enum class SizeSource : uint8_t
		{
			Constant,
			Varying
		};
		SizeSource get_size_source() const
		{
			return end_size.has_value() ? SizeSource::Varying : SizeSource::Constant;
		}

		glm::vec3 emit_position_min; // Position particles emit from.
		glm::vec3 emit_position_max; // Position particles emit from.

		glm::vec3 emit_velocity_min; // Minimum starting velocity in m/s.
		glm::vec3 emit_velocity_max; // Maximum starting velocity in m/s.

		glm::vec3 acceleration;

		DeltaTime lifetime_min; // Min duration in seconds a particle stays alive before being removed.
		DeltaTime lifetime_max; // Max duration in seconds a particle stays alive before being removed.

		float spawn_per_second; // How many particles to spawn every second.
		float spawn_debt;

		unsigned int max_particle_count; // Max number of particles that can be alive concurrently.
		unsigned int alive_count;        // Number of particles currently alive. This value is out of date when particles expire after particle_update kernel runs.

		OpenGL::Buffer particle_buf; // Contains instances of Particle struct.

		ParticleEmitter(const TextureRef& p_texture);
		void draw_UI(System::TextureSystem& p_texture_system);
		// Clear the particles and begin spawning them again.
		void reset();

		std::pair<TextureRef&, TextureRef&> get_textures() { return {start_texture, end_texture}; }
		std::pair<const TextureRef&, const TextureRef&> get_textures() const { return {start_texture, end_texture}; }
	};

	inline const char* to_string(ParticleEmitter::ColourSource p_source)
	{
		switch (p_source)
		{
			case ParticleEmitter::ColourSource::ConstantColour:               return "Constant colour";
			case ParticleEmitter::ColourSource::ConstantTexture:              return "Constant texture";
			case ParticleEmitter::ColourSource::ConstantColourAndTexture:     return "Constant colour and texture";
			case ParticleEmitter::ColourSource::VaryingColour:                return "Varying colour";
			case ParticleEmitter::ColourSource::VaryingTexture:               return "Varying texture";
			case ParticleEmitter::ColourSource::VaryingColourConstantTexture: return "Varying colour, constant texture";
			case ParticleEmitter::ColourSource::ConstantColourVaryingTexture: return "Constant colour, varying texture";
			case ParticleEmitter::ColourSource::VaryingColourAndTexture:      return "Varying colour and texture";
			default:                                                          return "Unknown";
		}
	}
}// namespace Component