#pragma once

#include "Texture.hpp"

#include "vector"
#include "string"

// Mesh stores all per-vertex data to represent a 3D object.
struct Mesh
{
    friend class MeshManager;

    std::string mName;

    std::vector<float> mVertices;           // Per-vertex position attributes.
    std::vector<float> mNormals;            // Per-vertex normal attributes.
    std::vector<float> mColours;            // Per-vertex colour attributes.
    std::vector<float> mTextureCoordinates; // Per-vertex texture mapping.
    std::vector<int> mIndices;              // Allows indexing into the mVertices and mColours data to specify an indexed draw order.

    // Composite mesh members:
    std::vector<TextureID>  mTextures;
    std::vector<Mesh>       mChildMeshes;
    std::string             mFilePath;

    MeshID getID() const { return mID; }
private:
    MeshID mID; // Unique ID assigned by MeshManager when a mesh is constructed.
    static inline MeshID nextMesh = 0;
};