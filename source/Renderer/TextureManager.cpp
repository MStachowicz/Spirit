#include "TextureManager.hpp"
#include "Logger.hpp"

#define STB_IMAGE_IMPLEMENTATION // This modifies the header such that it only contains the relevant definition source code
#include "stb_image.h"
#include "FileSystem.hpp"

TextureID TextureManager::getTextureID(const std::string &pTextureName) const
{
    const auto it = mNameLookup.find(pTextureName);
    ZEPHYR_ASSERT(it != mNameLookup.end(), "Searching for a texture that does not exist in Texture store.");

    return mTextures[it->second].getID();
}

std::string TextureManager::getTextureName(const TextureID& pTextureID) const
{
    ZEPHYR_ASSERT(pTextureID < mTextures.size(), "Searching for a texture off the end of Texture container.");
    return mTextures[pTextureID].mName;
}

TextureManager::TextureManager()
{
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

    // Load all the textures in the textures directory
    const std::vector<std::string> textureFileNames = File::getAllFileNames(File::textureDirectory);

    for (const auto& fileName : textureFileNames)
        loadTexture(fileName);
}

// Returns the texture data given a file name. Searches the File::textureDirectory for the texture.
// Uses stb_image to load the texture from file.
void TextureManager::loadTexture(const std::string& pFileName)
{
    const std::string path = File::textureDirectory + pFileName;
    ZEPHYR_ASSERT(File::exists(path), "The texture file with path {} could not be found.", path)

    Texture& newTexture =  mTextures[activeTextures];
    newTexture.mData = stbi_load(path.c_str(), &newTexture.mWidth, &newTexture.mHeight, &newTexture.mNumberOfChannels, 0);
    ZEPHYR_ASSERT(newTexture.mData != nullptr, "Failed to load texture");
    newTexture.mName = File::removeFileExtension(pFileName);
    newTexture.mID = activeTextures;

    mNameLookup.insert(std::make_pair(newTexture.mName, newTexture.mID));
    activeTextures++;
}