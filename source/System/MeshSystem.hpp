#pragma once

// Component
#include "Mesh.hpp"

// Utility
#include "ResourceManager.hpp"

// STD
#include <filesystem>

namespace System
{
    class TextureSystem;

    class MeshSystem
    {
        System::TextureSystem& mTextureSystem;

    public:
        MeshSystem(TextureSystem& pTextureSystem) noexcept;

        std::vector<std::filesystem::path> mAvailableModels;
        ModelManager mModelManager;

        ModelRef getModel(const std::filesystem::path& pFilePath);

        ModelRef mConePrimitive;
        ModelRef mCubePrimitive;
        ModelRef mCylinderPrimitive;
        ModelRef mPlanePrimitive;
        ModelRef mSpherePrimitive;
    };
} // namespace System