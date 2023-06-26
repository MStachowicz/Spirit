#include "TextureSystem.hpp"

#include "File.hpp"
#include "Utility.hpp"
#include "Config.hpp"

namespace System
{
    TextureSystem::TextureSystem() noexcept
        : mAvailableTextures{}
        , mTextureManager{}
    {
        Utility::File::forEachFile(Config::Texture_Directory, [&](auto& entry)
        {
            if (entry.is_regular_file())
                mAvailableTextures.push_back(entry.path());
        });
    }
} // namespace System