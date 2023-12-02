#include "TextureSystem.hpp"

#include "Utility/File.hpp"
#include "Utility/Utility.hpp"
#include "Utility/Config.hpp"

namespace System
{
	TextureSystem::TextureSystem() noexcept
		: m_available_textures{}
		, m_texture_manager{}
	{
		Utility::File::foreach_file(Config::Texture_Directory, [&](auto& entry)
		{
			if (entry.is_regular_file())
				m_available_textures.push_back(entry.path());
		});
	}
} // namespace System