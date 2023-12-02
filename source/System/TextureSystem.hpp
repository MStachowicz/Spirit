#pragma once

#include "Component/Texture.hpp"
#include "Utility/ResourceManager.hpp"

#include <filesystem>
#include <vector>

namespace System
{
	// Keeps track of all the available textures on file and owns all the Data::Textures via the m_texture_manager.
	// Acts as a factory for TextureRefs via getTexture(path).
	class TextureSystem
	{
	public:
		std::vector<std::filesystem::path> m_available_textures; // All the available texture files.
		TextureManager m_texture_manager;

		TextureSystem() noexcept;

		TextureRef getTexture(const std::filesystem::path& pFilePath)
		{
			return m_texture_manager.get_or_create([&pFilePath](const Data::Texture& pTexture)
			{
				return pTexture.m_image_ref->m_filepath == pFilePath;
			}, pFilePath);
		}
	};
} // namespace System