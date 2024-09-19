#pragma once

#include "OpenGL/Types.hpp"
#include "Utility/ResourceManager.hpp"
#include "Utility/File.hpp"

#include <glm/vec4.hpp>
#include <filesystem>

namespace Data
{
	// Texture represents an image file on disk and its associated GPU handle.
	// On construction a Texture is loaded into memory and onto the GPU ready for rendering.
	class Texture
	{
	public:
		Texture(const std::filesystem::path& p_filePath) noexcept;
		~Texture()                                     = default;
		Texture(Texture&& p_other) noexcept            = default;
		Texture& operator=(Texture&& p_other) noexcept = default;
		Texture(const Texture& p_other)                = delete;
		Texture& operator=(const Texture& p_other)     = delete;

		Utility::File::ImageRef m_image_ref;
		OpenGL::Texture m_GL_texture;
	};
}

using TextureManager = Utility::ResourceManager<Data::Texture>;
using TextureRef     = Utility::ResourceRef<Data::Texture>;

namespace System
{
	class AssetManager;
}
namespace Component
{
	class Texture
	{
	public:
		constexpr static size_t Persistent_ID = 2;

		TextureRef m_diffuse;
		TextureRef m_specular;
		float m_shininess;

		glm::vec4 m_colour;

		Texture() noexcept;
		Texture(const TextureRef& m_diffuse) noexcept;
		Texture(const glm::vec4& p_colour) noexcept;

		void draw_UI(System::AssetManager& p_asset_manager);
	};
}; // namespace Component