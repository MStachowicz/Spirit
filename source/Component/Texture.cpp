#include "Texture.hpp"

#include "Logger.hpp"
#include "File.hpp"

//#define STB_IMAGE_IMPLEMENTATION // This modifies the header such that it only contains the relevant definition source code
#include "stb_image.h"

namespace Data
{
    Texture::Texture(const std::filesystem::path& pFilePath)
        : mFilePath{pFilePath}
        , mName{pFilePath.stem().string()}
        , mWidth{-1}
        , mHeight{-1}
        , mNumberOfChannels{-1}
    {
        //ASSERT(Utility::File::exists(pFilePath), "The texture file not found at path '{}'", pFilePath.string());

        // OpenGL expects 0 coordinate on y-axis to be the bottom side of the image, images usually have 0 at the top of y-axis
        // Flip textures here to account for this.
        stbi_set_flip_vertically_on_load(true);
        mData = stbi_load(pFilePath.string().c_str(), &mWidth, &mHeight, &mNumberOfChannels, 0);
        ASSERT(mData != nullptr, "Failed to load texture");

        mGLTexture = OpenGL::Texture(*this);

        LOG("Data::Texture '{}' loaded", mName);
    }

    Texture::~Texture()
    {
        stbi_image_free(mData);
        LOG("Data::Texture '{}' destroyed", mName);
    }
}