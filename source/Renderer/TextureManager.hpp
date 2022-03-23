#pragma once

#include "Texture.hpp"

#include "array"
#include "unordered_map"
#include "functional"

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
private:
    size_t activeTextures = 0;

    std::array<Texture, MAX_TEXTURES> mTextures;
    std::unordered_map<std::string, TextureID> mNameLookup;

    // Searches File::textureDirectory for texture matching pFileName.
    // Uses stb_image to load the pixel data from file.
    void loadTexture(const std::string& pFileName);
};