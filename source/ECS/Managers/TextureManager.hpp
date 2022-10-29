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

namespace Manager
{
    class TextureManager
    {
    public:
        TextureManager();
        TextureID getTextureID(const std::string& pTextureName) const;
        std::string getTextureName(const TextureID& pTextureID) const;

        inline void ForEach(const std::function<void(const Data::Texture&)>& pFunction) const
        {
            for (const auto& Texture : mTextures)
                pFunction(Texture);
        }

        inline void ForEachCubeMap(const std::function<void(const Data::CubeMapTexture&)>& pFunction) const
        {
            for (size_t i = 0; i < mCubeMaps.size(); i++)
                pFunction(mCubeMaps[i]);
        }

        // Loads individual texture data at pFilePath using stb_image.
        // The Texture is added to the TextureManager store and the pointer to it returned.
        Data::Texture& loadTexture(const std::filesystem::path& pFilePath, const Data::Texture::Purpose pPurpose = Data::Texture::Purpose::Diffuse, const std::string& pName = "");
        // Loads all the cubemap textures. pCubeMapDirectory is the root of all the cubemaps individually storing 6 textures per folder.
        void loadCubeMaps(const std::filesystem::directory_entry& pCubeMapDirectory);

    private:
        std::vector<Data::Texture> mTextures;
        std::unordered_map<std::string, size_t> mNameLookup;
        std::unordered_map<std::string, size_t> mFilePathLookup;

        std::vector<Data::CubeMapTexture> mCubeMaps;
    };
} // namespace Manager