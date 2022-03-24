#pragma once

#include "vector"
#include "string"

typedef unsigned int MeshID; // The unique ID used as an identifier of a Mesh.

// Mesh stores all per-vertex data to represent a 3D object.
struct Mesh
{
    Mesh() : mID(++nextMesh) {}

    const MeshID mID; // Unique ID to map this mesh to DrawInfo within the graphics context being used.
    std::string mName;
    std::vector<std::string> mAttributes;

    std::vector<float> mVertices;           // Per-vertex position attributes.
    std::vector<float> mNormals;            // Per-vertex normal attributes.
    std::vector<float> mColours;            // Per-vertex colour attributes.
    std::vector<float> mTextureCoordinates; // Per-vertex texture mapping.
    std::vector<int> mIndices;              // Allows indexing into the mVertices and mColours data to specify an indexed draw order.
private:
    static inline MeshID nextMesh = 0;
};