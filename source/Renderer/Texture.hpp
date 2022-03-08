#pragma once

#include "string"
#include "unordered_map"
#include "functional"

typedef unsigned int TextureID;

struct Texture
{
    friend class TextureManager;

    Texture(const std::string& pName, const int& pWidth, const int& pHeight, const int& pNumberOfChannels, unsigned char* pData);
    ~Texture();

    const TextureID mID; // Unique ID to map this mesh to DrawInfo within the graphics context being used.
    std::string mName = "";
    int mWidth = -1;
    int mHeight = -1;
    int mNumberOfChannels = -1;

    const unsigned char* getData() const { return mData; }

private:
    unsigned char *mData; // Raw pointer deleted in ~Texture using stbi_image_free.
    static inline TextureID nextTexture = 0;

    // Frees the mData memory, must be called before destroying Texture
    void release();
};

class TextureManager
{
public:
    TextureManager();
    TextureID getTextureID(const std::string& pTextureName);

    inline void ForEach(const std::function<void(const Texture&)>& pFunction) const
    {
        for (const auto& texture : mTextures)
            pFunction(texture.second);
    }
private:
    std::unordered_map<TextureID, Texture> mTextures;
    std::unordered_map<std::string, TextureID> mNameLookup;

    // Returns the texture data given a file name. Searches the File::textureDirectory for the texture.
    // Uses stb_image to load the texture from file.
    void loadTexture(const std::string& pFileName);
};