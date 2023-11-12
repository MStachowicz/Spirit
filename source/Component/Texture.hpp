#pragma once

#include "OpenGL/Types.hpp"
#include "Utility/ResourceManager.hpp"
#include "Utility/File.hpp"

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
	class TextureSystem;
}
namespace Component
{
	class Texture
	{
	public:
		TextureRef mDiffuse;
		TextureRef mSpecular;
		float m_shininess;

		Texture() noexcept;
		Texture(const TextureRef& m_diffuse) noexcept;
		void draw_UI(System::TextureSystem& p_texture_system);
	};
}; // namespace Component