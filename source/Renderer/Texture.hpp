#pragma once

#include "string"

typedef size_t TextureID;

// Texture is a data only container used by TextureManager to store loaded textures.
struct Texture
{
    friend class TextureManager;

    enum class Purpose { Diffuse, Normal, Specular, Height };

    std::string mName       = "";
    std::string mFilePath   = "";
    int mWidth              = -1;
    int mHeight             = -1;
    int mNumberOfChannels   = -1;
    Purpose mPurpose        = Purpose::Diffuse;

    const unsigned char* getData() const { return mData; }
    TextureID getID() const { return mID; }
private:
    unsigned char* mData; // Raw pointer deleted in using stbi_image_free().
    TextureID mID            = 0;
};