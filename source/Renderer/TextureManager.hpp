#pragma once

#include "Texture.hpp"

#include "array"
#include "unordered_map"
#include "functional"

namespace std
{
    namespace filesystem
    {
        class path;
    }
}

class TextureManager
{
public:
    const inline static size_t MAX_TEXTURES = 50;

    TextureManager();
    TextureID getTextureID(const std::string& pTextureName) const;
    std::string getTextureName(const TextureID& pTextureID) const;

    inline void ForEach(const std::function<void(const Texture&)>& pFunction) const
    {
        for (size_t i = 0; i < activeTextures; i++)
            pFunction(mTextures[i]);
    }

    // Loads a texture from pFilePath using stb_image. Returns the TextureID for the loaded Texture.
    TextureID loadTexture(const std::filesystem::path& pFilePath, const Texture::Purpose pPurpose = Texture::Purpose::Diffuse, const std::string& pName = "");
private:
    size_t activeTextures = 0;

    std::array<Texture, MAX_TEXTURES> mTextures;
    std::unordered_map<std::string, TextureID> mNameLookup;
    std::unordered_map<std::string, TextureID> mFilePathLookup;
};