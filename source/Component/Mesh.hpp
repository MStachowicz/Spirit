#pragma once

#include "Texture.hpp"

#include "glm/vec3.hpp"

#include <vector>
#include <string>
#include <optional>

namespace Component
{
    // Unique ID of a mesh, represents the index of the Mesh in the Mesh container.
    struct MeshID
    {
        size_t Get() const { return mID.value(); }
        void Set(size_t pID) { mID = pID; }

        bool operator==(const Component::MeshID& rhs) const { return mID == rhs.mID; }

    private:
        std::optional<size_t> mID;
    };

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
        Component::MeshID mID;
        std::string mName;
        std::string mFilePath;

        std::vector<float> mVertices;           // Per-vertex position attributes.
        std::vector<float> mNormals;            // Per-vertex normal attributes.
        std::vector<float> mColours;            // Per-vertex colour attributes.
        std::vector<float> mTextureCoordinates; // Per-vertex texture mapping.
        std::vector<int> mIndices;              // Allows indexing into the mVertices and mColours data to specify an indexed draw order.

        // Composite mesh members:
        std::vector<Component::TextureID> mTextures;
        std::vector<Mesh> mChildMeshes;
    };


    // Represents a reference to a mesh owned by the MeshSystem.
    // Also stores information on how the mesh wants to be drawn.
    // Used as a component for Entities without the data implications of storing all the per-vertex info.
    struct MeshDraw
    {
        Component::MeshID mID;
        std::string mName;
        DrawMode mDrawMode      = Component::DrawMode::Fill;
        DrawStyle mDrawStyle    = Component::DrawStyle::Textured;

        // DrawStyle::Textured
        std::optional<Component::TextureID> mTexture1          = std::nullopt;
        std::optional<Component::TextureID> mTexture2          = std::nullopt;
        std::optional<float> mMixFactor             = std::nullopt; // If mTexture1 and mTexture2 are set, allows setting the balance between the two textures.

        // DrawStyle::UniformColour
        std::optional<glm::vec3> mColour            = std::nullopt;

       // DrawStyle::LightMap
        std::optional<Component::TextureID> mDiffuseTextureID  = std::nullopt;
        std::optional<Component::TextureID> mSpecularTextureID = std::nullopt;
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