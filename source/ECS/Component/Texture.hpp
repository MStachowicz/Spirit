#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>

namespace System
{
    class TextureSystem;
}

namespace Component
{
    // Unique ID of a texture, represents the index of the Texture in the Texture container.
    struct TextureID
    {
        size_t Get() const { return mID.value(); }
        void Set(size_t pID) { mID = pID; }

        bool operator==(const TextureID& rhs) const { return mID == rhs.mID; }

    private:
        std::optional<size_t> mID;
    };

    // Texture represents a texture file on the disk. Texture are created and owned by TextureSystem to store loaded textures.
    struct Texture
    {
        friend class System::TextureSystem;

        TextureID mID;
        std::string mName               = "";
        std::filesystem::path mFilePath = "";

        int mWidth            = -1;
        int mHeight           = -1;
        int mNumberOfChannels = -1;
        enum class Purpose { Diffuse, Normal, Specular, Height, Cubemap, None };
        Purpose mPurpose = Purpose::None;

        const unsigned char* getData() const { return mData; }

    private:
        unsigned char* mData; // Raw pointer deleted in using stbi_image_free().
    };

    // Represents 6 textures defining the faces of a cube.
    struct CubeMapTexture
    {
        std::string mName = "";
        std::filesystem::path mFilePath;

        Texture mRight;
        Texture mLeft;
        Texture mTop;
        Texture mBottom;
        Texture mBack;
        Texture mFront;
    };
}; // namespace Component