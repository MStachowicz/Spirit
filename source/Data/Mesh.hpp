#pragma once

#include "Texture.hpp"

#include "Utility.hpp"

#include "vector"
#include "string"
#include "optional"

namespace Data
{
    enum class DrawStyle : size_t
    {
        Textured = 0,
        UniformColour = 1,
        LightMap = 2,

        Count
    };

    enum class DrawMode
    {
        Fill,
        Wireframe,

        Count
    };

    // Allows iterating over enum class DrawMode
    static const std::array<std::string, util::toIndex(DrawMode::Count)> drawModes{"Fill", "Wireframe"};
    static std::string convert(const DrawMode &pDrawMode) { return drawModes[util::toIndex(pDrawMode)]; }
    // Allows iterating over enum class DrawStyle
    static const std::array<std::string, util::toIndex(DrawStyle::Count)> drawStyles{"Textured", "Uniform Colour", "Light Map"};
    static std::string convert(const DrawStyle &pDrawStyle) { return drawStyles[util::toIndex(pDrawStyle)]; }

    // Mesh stores all per-vertex data to represent a 3D object.
    struct Mesh
    {
        std::string mName;

        std::vector<float> mVertices;           // Per-vertex position attributes.
        std::vector<float> mNormals;            // Per-vertex normal attributes.
        std::vector<float> mColours;            // Per-vertex colour attributes.
        std::vector<float> mTextureCoordinates; // Per-vertex texture mapping.
        std::vector<int> mIndices;              // Allows indexing into the mVertices and mColours data to specify an indexed draw order.

        // Composite mesh members:
        std::vector<TextureID> mTextures;
        std::vector<Mesh> mChildMeshes;
        std::string mFilePath;

        MeshID getID() const { return mID; }
        void setID(const MeshID pID) { mID = pID; }

    private:
        MeshID mID; // Unique ID assigned by MeshManager when a mesh is constructed.
        static inline MeshID nextMesh = 0;
    };


    // Represents a reference to a mesh owned by the MeshManager.
    // Also stores information on how the mesh wants to be drawn.
    // Used as a component for Entities without the data implications of storing all the per-vertex info.
    struct MeshDraw
    {
        MeshID mID = 0;
        std::string mName;
        DrawMode mDrawMode      = Data::DrawMode::Fill;
        DrawStyle mDrawStyle    = Data::DrawStyle::Textured;

        // DrawStyle::Textured
        std::optional<TextureID> mTexture1          = std::nullopt;
        std::optional<TextureID> mTexture2          = std::nullopt;
        std::optional<float> mMixFactor             = std::nullopt; // If mTexture1 and mTexture2 are set, allows setting the balance between the two textures.

        // DrawStyle::UniformColour
        std::optional<glm::vec3> mColour            = std::nullopt;

       // DrawStyle::LightMap
        std::optional<TextureID> mDiffuseTextureID  = std::nullopt;
        std::optional<TextureID> mSpecularTextureID = std::nullopt;
        std::optional<float> mShininess             = std::nullopt;

        std::optional<float> mTextureRepeatFactor   = std::nullopt;

        void DrawImGui() {};
    };
}