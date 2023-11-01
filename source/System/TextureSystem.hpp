#pragma once

// COMPONENT
#include "Texture.hpp"

// UTILITY
#include "ResourceManager.hpp"

// STD
#include <filesystem>
#include <vector>

namespace System
{
    // Keeps track of all the available textures on file and owns all the Data::Textures via the mTextureManager.
    // Acts as a factory for TextureRefs via getTexture(path).
    class TextureSystem
    {
    public:
        std::vector<std::filesystem::path> mAvailableTextures; // All the available texture files.
        TextureManager mTextureManager;

        TextureSystem() noexcept;

        TextureRef getTexture(const std::filesystem::path& pFilePath)
        {
            return mTextureManager.get_or_create([&pFilePath](const Data::Texture& pTexture)
            {
                return pTexture.m_image_ref->m_filepath == pFilePath;
            }, pFilePath);
        }
    };
} // namespace System