#pragma once

#include <optional>

// Unique ID of a mesh, represents the index of the Mesh in the Mesh container.
struct MeshID
{
    size_t Get() const { return mID.value(); }
    void Set(size_t pID) { mID = pID; }

    bool operator==(const MeshID &rhs) const { return mID == rhs.mID; }

private:
    std::optional<size_t> mID;
};

// Unique ID of a texture, represents the index of the Texture in the Texture container.
struct TextureID
{
    size_t Get() const { return mID.value(); }
    void Set(size_t pID) { mID = pID; }

    bool operator==(const TextureID &rhs) const { return mID == rhs.mID; }
private:
    std::optional<size_t> mID;
};