#pragma once

#include "Types.hpp"

#include "filesystem"
#include "string"
#include "array"

// Texture is a data only container used by TextureManager to store loaded textures.
struct Texture
{
    friend class TextureManager;

    enum class Purpose { Diffuse, Normal, Specular, Height, Cubemap, None };

    std::string mName       = "";
    std::string mFilePath   = "";
    int mWidth              = -1;
    int mHeight             = -1;
    int mNumberOfChannels   = -1;
    Purpose mPurpose        = Purpose::None;

    const unsigned char* getData() const { return mData; }
    TextureID getID() const { return mID; }
private:
    unsigned char* mData; // Raw pointer deleted in using stbi_image_free().
    TextureID mID            = 0;
};

// Represents 6 textures defining the faces of a cube.
struct CubeMapTexture
{
    std::string mName       = "";
    std::filesystem::path mFilePath;
    std::array<TextureID, 6> textures = {0, 0, 0, 0, 0, 0};
};