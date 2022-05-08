#include "TextureManager.hpp"
#include "Logger.hpp"

#define STB_IMAGE_IMPLEMENTATION // This modifies the header such that it only contains the relevant definition source code
#include "stb_image.h"
#include "FileSystem.hpp"
#include "Utility.hpp"

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
    util::File::ForEachFile(util::File::textureDirectory, [&](auto& entry)
    {
        if (entry.is_regular_file())
            loadTexture(entry.path()); // Load all the texture files in the root texture folder.
        else if (entry.is_directory() && entry.path().stem().string() == "Cubemaps")
            loadCubeMaps(entry); // Load textures in the Cubemaps directory
    });
}

void TextureManager::loadCubeMaps(const std::filesystem::directory_entry& pCubeMapsDirectory)
{
    // Iterate over every folder inside pCubeMapsDirectory, for each folder iterate over 6 textures to load individual
    // texture data into a CubeMapTexture object.
    util::File::ForEachFile(pCubeMapsDirectory, [&](auto &cubemapDirectory)
    {
        ZEPHYR_ASSERT(cubemapDirectory.is_directory(), "Path given was not a directory. Store cubemaps in folders.");
        CubeMapTexture cubemap;
        cubemap.mName = cubemapDirectory.path().stem().string();
        cubemap.mFilePath = cubemapDirectory.path();

        int count = 0;
        util::File::ForEachFile(cubemapDirectory, [&](auto& cubemapTexture)
        {
            ZEPHYR_ASSERT(cubemapTexture.is_regular_file(), "Cubemap directory contains non-texture files.");

            const std::string textureName = cubemapTexture.path().stem().string();

            // The loadTexture calls below push the texture to the TextureManager and then are also copied into the cubemap.
            if (textureName == "right")
                cubemap.mRight = loadTexture(cubemapTexture.path(), Texture::Purpose::Cubemap);
            else if (textureName == "left")
                cubemap.mLeft = loadTexture(cubemapTexture.path(), Texture::Purpose::Cubemap);
            else if (textureName == "top")
                cubemap.mTop = loadTexture(cubemapTexture.path(), Texture::Purpose::Cubemap);
            else if (textureName == "bottom")
                cubemap.mBottom = loadTexture(cubemapTexture.path(), Texture::Purpose::Cubemap);
            else if (textureName == "back")
                cubemap.mBack = loadTexture(cubemapTexture.path(), Texture::Purpose::Cubemap);
            else if (textureName == "front")
                cubemap.mFront = loadTexture(cubemapTexture.path(), Texture::Purpose::Cubemap);

            count++;
        });

        ZEPHYR_ASSERT(count == 6, "There must be 6 loaded textures for a cubemap.");
        mCubeMaps.push_back(cubemap);
        LOG_INFO("Zephyr::Cubemap '{}' loaded", cubemap.mName);
    });
}

Texture& TextureManager::loadTexture(const std::filesystem::path& pFilePath, const Texture::Purpose pPurpose, const std::string& pName/* = "" */)
{
    if (pPurpose == Texture::Purpose::Cubemap)
        stbi_set_flip_vertically_on_load(false);
    else
        stbi_set_flip_vertically_on_load(true);

    ZEPHYR_ASSERT(File::exists(pFilePath.string()), "The texture file with path {} could not be found.", pFilePath)

    const auto& textureLocation = mFilePathLookup.find(pFilePath.string());
    if (textureLocation != mFilePathLookup.end())
    {
        // If the texture in this location has been loaded before, skip load and return the Texture.
        return mTextures[textureLocation->second];
    }
    else
    {
        Texture& newTexture = mTextures[activeTextures];
        newTexture.mData = stbi_load(pFilePath.string().c_str(), &newTexture.mWidth, &newTexture.mHeight, &newTexture.mNumberOfChannels, 0);
        ZEPHYR_ASSERT(newTexture.mData != nullptr, "Failed to load texture");

        newTexture.mName = pName.empty() ? pFilePath.stem().string() : pName;
        newTexture.mFilePath = pFilePath;
        newTexture.mPurpose = pPurpose;
        newTexture.mID = activeTextures;

        ZEPHYR_ASSERT(mNameLookup.find(newTexture.mName) == mNameLookup.end(), "Name has to be unique");
        mNameLookup.insert(std::make_pair(newTexture.mName, newTexture.mID));
        mFilePathLookup.insert(std::make_pair(newTexture.mFilePath.string(), newTexture.mID));

        activeTextures++;
        ZEPHYR_ASSERT(activeTextures == mNameLookup.size(), "NameLookup should have parity with mTextures size");
        ZEPHYR_ASSERT(activeTextures == mFilePathLookup.size(), "FilePathLookup should have parity with mTextures size");
        LOG_INFO("Zephyr::Texture '{}' loaded given TextureID {}", newTexture.mName, newTexture.mID);
        return newTexture;
    }
}