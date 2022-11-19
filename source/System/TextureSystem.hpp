#pragma once

#include "Texture.hpp"

#include <functional>
#include <unordered_map>
#include <vector>

namespace std
{
    namespace filesystem
    {
        class path;
        class directory_entry;
    } // namespace filesystem
} // namespace std

namespace System
{
    class TextureSystem
    {
    public:
        TextureSystem();
        Component::TextureID getTextureID(const std::string& pTextureName) const;
        std::string getTextureName(const Component::TextureID& pTextureID) const;

        inline void ForEach(const std::function<void(const Component::Texture&)>& pFunction) const
        {
            for (const auto& Texture : mTextures)
                pFunction(Texture);
        }

        inline void ForEachCubeMap(const std::function<void(const Component::CubeMapTexture&)>& pFunction) const
        {
            for (size_t i = 0; i < mCubeMaps.size(); i++)
                pFunction(mCubeMaps[i]);
        }

        // Loads individual texture data at pFilePath using stb_image.
        // The Texture is added to the TextureSystem store and the pointer to it returned.
        Component::Texture& loadTexture(const std::filesystem::path& pFilePath, const Component::Texture::Purpose pPurpose = Component::Texture::Purpose::Diffuse, const std::string& pName = "");
        // Loads all the cubemap textures. pCubeMapDirectory is the root of all the cubemaps individually storing 6 textures per folder.
        void loadCubeMaps(const std::filesystem::directory_entry& pCubeMapDirectory);

    private:
        std::vector<Component::Texture> mTextures;
        std::unordered_map<std::string, size_t> mNameLookup;
        std::unordered_map<std::string, size_t> mFilePathLookup;

        std::vector<Component::CubeMapTexture> mCubeMaps;
    };
} // namespace System