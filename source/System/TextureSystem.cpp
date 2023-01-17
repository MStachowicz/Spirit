#include "TextureSystem.hpp"

#include "File.hpp"
#include "Utility.hpp"

namespace System
{
    TextureSystem::TextureSystem() noexcept
        : mAvailableTextures{}
        , mTextureManager{}
    {
        Utility::File::forEachFile(Utility::File::textureDirectory, [&](auto& entry)
        {
            if (entry.is_regular_file())
                mAvailableTextures.push_back(entry.path());
        });
    }
} // namespace System