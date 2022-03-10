#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION // This modifies the header such that it only contains the relevant definition source code
#include "stb_image.h"

#include "FileSystem.hpp"
#include "Logger.hpp"

Texture::Texture(const std::string& pName, const int& pWidth, const int& pHeight, const int& pNumberOfChannels, unsigned char* pData)
    : mID(++nextTexture)
    , mName(pName)
    , mWidth(pWidth)
    , mHeight(pHeight)
    , mNumberOfChannels(pNumberOfChannels)
    , mData(pData)
{
}

Texture::~Texture()
{}

void Texture::release()
{
    stbi_image_free(mData);
}

TextureID TextureManager::getTextureID(const std::string &pTextureName) const
{
    const auto it = mNameLookup.find(pTextureName);
    ZEPHYR_ASSERT(it != mNameLookup.end(), "Searching for a texture that does not exist in Texture store.");
    return it->second;
}

std::string TextureManager::getTextureName(const TextureID& pTextureID) const
{
    const auto it = mTextures.find(pTextureID);
    ZEPHYR_ASSERT(it != mTextures.end(), "Searching for a texture that does not exist in Texture store.");
    return it->second.mName;
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

    int width, height, numberOfChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &numberOfChannels, 0);
    ZEPHYR_ASSERT(data != nullptr, "Failed to load texture")

    Texture texture = Texture(File::removeFileExtension(pFileName), width, height, numberOfChannels, data);
    mTextures.insert(std::make_pair(texture.mID, texture));
    mNameLookup.insert(std::make_pair(texture.mName, texture.mID));
}