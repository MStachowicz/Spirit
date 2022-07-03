#pragma once

#include "Types.hpp"

#include <filesystem>
#include <string>
#include <array>

// Texture is a data only container used by TextureManager to store loaded textures.
struct Texture
{
    friend class TextureManager;

    TextureID mID;
    std::string mName               = "";
    std::filesystem::path mFilePath = "";

    int mWidth                      = -1;
    int mHeight                     = -1;
    int mNumberOfChannels           = -1;
    enum class Purpose { Diffuse, Normal, Specular, Height, Cubemap, None };
    Purpose mPurpose                = Purpose::None;

    const unsigned char* getData() const { return mData; }

private:
    unsigned char* mData; // Raw pointer deleted in using stbi_image_free().
};

// Represents 6 textures defining the faces of a cube.
struct CubeMapTexture
{
    std::string mName       = "";
    std::filesystem::path mFilePath;

    Texture mRight;
    Texture mLeft;
    Texture mTop;
    Texture mBottom;
    Texture mBack;
    Texture mFront;
};