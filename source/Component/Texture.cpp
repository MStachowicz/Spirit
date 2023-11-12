#include "Texture.hpp"
#include "System/TextureSystem.hpp"

#include "Utility/Logger.hpp"
#include "Utility/File.hpp"

#include "imgui.h"

namespace Data
{
	Texture::Texture(const std::filesystem::path& p_filepath) noexcept
		: m_image_ref{Utility::File::s_image_files.get_or_create([&p_filepath](const Utility::Image& p_image){ return p_image.m_filepath == p_filepath; }, p_filepath)}
		, m_GL_texture{*m_image_ref}
	{
		LOG("Data::Texture '{}' loaded", m_image_ref->m_filepath.string());
	}
}

namespace Component
{
	Texture::Texture() noexcept
		: mDiffuse{}
		, mSpecular{}
		, m_shininess{32.f}
	{}
	Texture::Texture(const TextureRef& m_diffuse) noexcept
		: mDiffuse{m_diffuse}
		, mSpecular{}
		, m_shininess{32.f}
	{}

	void Texture::draw_UI(System::TextureSystem& p_texture_system)
	{
		if (ImGui::TreeNode("Texture"))
		{
			const std::string currentDiffuse  = mDiffuse ? mDiffuse->m_image_ref->name() : "None";
			const std::string currentSpecular = mSpecular ? mSpecular->m_image_ref->name() : "None";

			static size_t selected;
			auto& availableTextures = p_texture_system.mAvailableTextures;
			std::vector<std::string> availableTextureNames;
			for (const auto& path : availableTextures)
				availableTextureNames.push_back(path.stem().string());

			if (ImGui::ComboContainer("Diffuse Texture", currentDiffuse.c_str(), availableTextureNames, selected))
				mDiffuse = p_texture_system.getTexture(availableTextures[selected]);
			if (ImGui::ComboContainer("Specular Texture", currentSpecular.c_str(), availableTextureNames, selected))
				mSpecular = p_texture_system.getTexture(availableTextures[selected]);
			ImGui::Slider("Shininess", m_shininess, 1.f, 512.f, "%.1f");

			ImGui::TreePop();
		}
	}
} // namespace Component