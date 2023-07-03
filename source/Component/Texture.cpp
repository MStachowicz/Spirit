#include "Texture.hpp"

#include "Logger.hpp"
#include "File.hpp"

//#define STB_IMAGE_IMPLEMENTATION // This modifies the header such that it only contains the relevant definition source code
#include "stb_image.h"

namespace Data
{
    Texture::Texture(const std::filesystem::path& p_filepath)
        : m_image_ref{Utility::File::s_image_files.getOrCreate([&p_filepath](const Utility::Image& p_image){ return p_image.m_filepath == p_filepath; }, p_filepath)}
        , m_GL_texture{*m_image_ref}
    {
        LOG("Data::Texture '{}' loaded", m_image_ref->m_filepath.string());
    }
}

namespace Component
{
    Texture::Texture() noexcept
        : mDiffuse{std::nullopt}
        , mSpecular{std::nullopt}
        , m_shininess{32.f}
    {}
    Texture::Texture(const TextureRef& m_diffuse) noexcept
        : mDiffuse{m_diffuse}
        , mSpecular{std::nullopt}
        , m_shininess{32.f}
    {}
}