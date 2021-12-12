#pragma once

#include "string"
#include "vector"

class File
{
public:
    struct Texture
    {
        ~Texture();

        int mWidth = -1;
        int mHeight = -1;
        int mNumberOfChannels = -1;
        unsigned char *mData; // Raw pointer deleted in ~Texture using stbi_image_free.
    };

    static std::string executablePath;
    static std::string rootDirectory;
    static std::string shaderDirectory;
    static std::string textureDirectory;

    static std::string readFromFile(const std::string& pPath);
    // Returns the texture data given a file name. Searches the File::textureDirectory for the texture.
    // Uses stb_image to load the texture from file.
    static Texture getTexture(const std::string &pFileName);
    static std::vector<std::string> getAllFileNames(const std::string& pDirectory);
    static std::vector<std::string> getAllFilePaths(const std::string& pDirectory);
    static bool exists(const std::string& pPath);

    static void setupDirectories(const std::string& pExecutePath);
};