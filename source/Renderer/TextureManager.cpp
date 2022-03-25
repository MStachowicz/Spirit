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
    const auto files = File::getFiles(File::textureDirectory);

    for (const auto& file : files)
        loadTexture(file.path());
}

TextureID TextureManager::loadTexture(const std::filesystem::path& pFilePath, const Texture::Purpose pPurpose, const std::string& pName/* = "" */)
{
    ZEPHYR_ASSERT(File::exists(pFilePath.string()), "The texture file with path {} could not be found.", pFilePath)

    const auto& textureLocation = mFilePathLookup.find(pFilePath.string());
    if (textureLocation != mFilePathLookup.end())
    {
        // If the texture in this location has been loaded before, skip load and return the same TextureID.
        return textureLocation->second;
    }
    else
    {
        Texture &newTexture = mTextures[activeTextures];
        newTexture.mData = stbi_load(pFilePath.string().c_str(), &newTexture.mWidth, &newTexture.mHeight, &newTexture.mNumberOfChannels, 0);
        ZEPHYR_ASSERT(newTexture.mData != nullptr, "Failed to load texture");

        if (!pName.empty())
            newTexture.mName = pName;
        else
            newTexture.mName = pFilePath.stem().string();
        newTexture.mFilePath = pFilePath.string();
        newTexture.mPurpose = pPurpose;
        newTexture.mID = activeTextures;

        ZEPHYR_ASSERT(mNameLookup.find(newTexture.mName) == mNameLookup.end(), "Name has to be unique");
        mNameLookup.insert(std::make_pair(newTexture.mName, newTexture.mID));
        mFilePathLookup.insert(std::make_pair(newTexture.mFilePath, newTexture.mID));

        activeTextures++;
        ZEPHYR_ASSERT(activeTextures == mNameLookup.size(), "NameLookup should have parity with mTextures size");
        ZEPHYR_ASSERT(activeTextures == mFilePathLookup.size(), "FilePathLookup should have parity with mTextures size");
        return newTexture.mID;
    }
}