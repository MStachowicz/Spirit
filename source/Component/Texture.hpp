#pragma once

#include "Data/Image.hpp"
#include "OpenGL/Types.hpp"
#include "Utility/File.hpp"
#include "Utility/ResourceManager.hpp"

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <filesystem>
#include <string>

namespace Data
{
	// Texture represents an image file on disk and its associated GPU handle.
	// On construction a Texture is loaded into memory and onto the GPU ready for rendering.
	class Texture
	{
		std::filesystem::path m_filepath;
		Data::Image m_image;

	public:
		OpenGL::Texture m_GL_texture;

		Texture(const std::filesystem::path& p_filePath) noexcept;
		~Texture()                                     = default;
		Texture(Texture&& p_other) noexcept            = default;
		Texture& operator=(Texture&& p_other) noexcept = default;
		Texture(const Texture& p_other)                = delete;
		Texture& operator=(const Texture& p_other)     = delete;

		// Raw access to the image pixel data. Access for read only.
		std::byte* data() const { return m_image.data; }
		// Return a display-friendly name for this image.
		std::string name() const { return m_filepath.stem().string(); };
		// Return the resolution of the image in pixels.
		glm::uvec2 resolution() const { return {m_image.width, m_image.height}; }
		// Return the filepath of the image.
		const std::filesystem::path& filepath() const { return m_filepath; }
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