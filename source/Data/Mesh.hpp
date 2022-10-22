#pragma once

#include "Texture.hpp"

#include "Utility.hpp"

#include <vector>
#include <string>
#include <optional>

namespace Data
{
    enum class DrawStyle
    {
        Textured,
        UniformColour,
        LightMap,

        Count
    };
    enum class DrawMode
    {
        Fill,
        Wireframe,

        Count
    };

    // Mesh stores all per-vertex data to represent a 3D object.
    struct Mesh
    {
        MeshID mID;
        std::string mName;
        std::string mFilePath;

        std::vector<float> mVertices;           // Per-vertex position attributes.
        std::vector<float> mNormals;            // Per-vertex normal attributes.
        std::vector<float> mColours;            // Per-vertex colour attributes.
        std::vector<float> mTextureCoordinates; // Per-vertex texture mapping.
        std::vector<int> mIndices;              // Allows indexing into the mVertices and mColours data to specify an indexed draw order.

        // Composite mesh members:
        std::vector<TextureID> mTextures;
        std::vector<Mesh> mChildMeshes;
    };


    // Represents a reference to a mesh owned by the MeshManager.
    // Also stores information on how the mesh wants to be drawn.
    // Used as a component for Entities without the data implications of storing all the per-vertex info.
    struct MeshDraw
    {
        MeshID mID;
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

        bool operator== (const MeshDraw& pOther) const
        {
            return mID == pOther.mID &&
                mName == pOther.mName &&
                mDrawMode == pOther.mDrawMode &&
                mDrawStyle == pOther.mDrawStyle &&
                mTexture1 == pOther.mTexture1 &&
                mTexture2 == pOther.mTexture2 &&
                mMixFactor == pOther.mMixFactor &&
                mColour == pOther.mColour &&
                mDiffuseTextureID == pOther.mDiffuseTextureID &&
                mSpecularTextureID == pOther.mSpecularTextureID &&
                mShininess == pOther.mShininess &&
                mTextureRepeatFactor == pOther.mTextureRepeatFactor;
        };
        bool operator != (const MeshDraw& pOther) const { return !(*this == pOther); }

        void DrawImGui();
    };
}