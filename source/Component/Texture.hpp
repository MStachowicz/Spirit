#pragma once

// OPENGL
#include "Types.hpp"

// UTILITY
#include "ResourceManager.hpp"

// STD
#include <filesystem>
#include <optional>
#include <string>

namespace System
{
    class TextureSystem;
}

namespace Data
{
    // Represents a texture file on the disk. Constructing a Texture with a valid filepath to a texture file will load it using stb_image.
    // Texture also holds handle for the GPU data.
    class Texture
    {
    public:
        Texture(const std::filesystem::path& pFilePath);
        ~Texture();

        std::filesystem::path mFilePath;
        std::string mName;
        int mWidth;
        int mHeight;
        int mNumberOfChannels;

        OpenGL::Texture mGLTexture;

        const unsigned char* getData() const { return mData; }

    private:
        unsigned char* mData; // Raw pointer initialised and deleted Texture constructor and destructor using stbi.
    };
}

using TextureManager = Utility::ResourceManager<Data::Texture>;
using TextureRef     = Utility::ResourceRef<Data::Texture>;

namespace Component
{
    class Texture
    {
    public:
        std::optional<TextureRef> mDiffuse;
        std::optional<TextureRef> mSpecular;
    };
}; // namespace Component