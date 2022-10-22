#include "Mesh.hpp"

#include "imgui.h"

namespace Data
{
    void MeshDraw::DrawImGui()
    {
        if(ImGui::TreeNode(mName.empty() ? "Mesh" : (mName + "mesh").c_str()))
        {
            ImGui::TreePop();
        }

        // static const std::array<std::string, util::toIndex(DrawMode::Count)> drawModes{"Fill", "Wireframe"};
        // static std::string convert(const DrawMode &pDrawMode) { return drawModes[util::toIndex(pDrawMode)]; }
        // static const std::array<std::string, util::toIndex(DrawStyle::Count)> drawStyles{"Textured", "Uniform Colour", "Light Map"};
        // static std::string convert(const DrawStyle &pDrawStyle) { return drawStyles[util::toIndex(pDrawStyle)]; }

    //    { // Draw mode selection
    //        if (ImGui::BeginCombo("Draw Mode", convert(pDrawCall.mDrawMode).c_str(), ImGuiComboFlags()))
    //        {
    //            for (size_t i = 0; i < drawModes.size(); i++)
    //            {
    //                if (ImGui::Selectable(drawModes[i].c_str()))
    //                    pDrawCall.mDrawMode = static_cast<DrawMode>(i);
    //            }
    //            ImGui::EndCombo();
    //        }
    //    }
    //    { // Draw style selection
    //        if (ImGui::BeginCombo("Draw Style", convert(pDrawCall.mDrawStyle).c_str(), ImGuiComboFlags()))
    //        {
    //            for (size_t i = 0; i < drawStyles.size(); i++)
    //            {
    //                if (ImGui::Selectable(drawStyles[i].c_str()))
    //                    pDrawCall.mDrawStyle = static_cast<DrawStyle>(i);
    //            }
    //            ImGui::EndCombo();
    //        }
    //    }
    //    ImGui::Separator();
    //    switch (pDrawCall.mDrawStyle)
    //    {
    //    case Data::DrawStyle::Textured:
    //    {
    //        { // Texture 1
    //            const std::string currentTexture = pDrawCall.mTexture1.has_value() ? mTextureManager.getTextureName(pDrawCall.mTexture1.value()) : "Empty";
    //            if (ImGui::BeginCombo("Texture", currentTexture.c_str(), ImGuiComboFlags()))
    //            {
    //                mTextureManager.ForEach([&](const Texture &texture)
    //                                        {
	//			if (ImGui::Selectable(texture.mName.c_str()))
	//			{
	//				pDrawCall.mTexture1 = texture.getID();
	//			} });
    //                ImGui::EndCombo();
    //            }
    //        }
    //        if (pDrawCall.mTexture1.has_value())
    //        { // Texture 2
    //            const std::string currentTexture = pDrawCall.mTexture2.has_value() ? mTextureManager.getTextureName(pDrawCall.mTexture2.value()) : "Empty";
    //            if (ImGui::BeginCombo("Texture 2", currentTexture.c_str(), ImGuiComboFlags()))
    //            {
    //                if (pDrawCall.mTexture2.has_value())
    //                    if (ImGui::Selectable("Empty"))
    //                        pDrawCall.mTexture2 = std::nullopt;
    //                mTextureManager.ForEach([&](const Texture &texture)
    //                                        {
	//			if (ImGui::Selectable(texture.mName.c_str()))
	//			{
	//				pDrawCall.mTexture2 = texture.getID();
	//			} });
    //                ImGui::EndCombo();
    //            }
    //        }
    //        if (pDrawCall.mTexture1.has_value() && pDrawCall.mTexture2.has_value())
    //        { // Only displayed if we have two texture slots set
    //            if (!pDrawCall.mMixFactor.has_value())
    //                pDrawCall.mMixFactor = 0.5f;
    //            ImGui::SliderFloat("Texture mix factor", &pDrawCall.mMixFactor.value(), 0.f, 1.f);
    //        }
    //    }
    //    break;
    //    case Data::DrawStyle::UniformColour:
    //    {
    //        if (!pDrawCall.mColour.has_value())
    //            pDrawCall.mColour = glm::vec3(1.f, 1.f, 1.f);
    //        ImGui::ColorEdit3("Colour", &pDrawCall.mColour.value().x);
    //    }
    //    break;
    //    case Data::DrawStyle::LightMap:
    //    {
    //        ImGui::Text("Available texture slots");
    //        {
    //            const std::string currentTexture = pDrawCall.mDiffuseTextureID.has_value() ? mTextureManager.getTextureName(pDrawCall.mDiffuseTextureID.value()) : "No texture set";
    //            if (ImGui::BeginCombo("Diffuse", currentTexture.c_str(), ImGuiComboFlags()))
    //            {
    //                mTextureManager.ForEach([&](const Texture &texture)
    //                                        {
	//			if (ImGui::Selectable(texture.mName.c_str()))
	//				pDrawCall.mDiffuseTextureID = texture.getID(); });
    //                ImGui::EndCombo();
    //            }
    //        }
    //        {
    //            const std::string currentTexture = pDrawCall.mSpecularTextureID.has_value() ? mTextureManager.getTextureName(pDrawCall.mSpecularTextureID.value()) : "No texture set";
    //            if (ImGui::BeginCombo("Specular", currentTexture.c_str(), ImGuiComboFlags()))
    //            {
    //                mTextureManager.ForEach([&](const Texture &texture)
    //                                        {
	//			if (ImGui::Selectable(texture.mName.c_str()))
	//				pDrawCall.mSpecularTextureID = texture.getID(); });
    //                ImGui::EndCombo();
    //            }
    //        }
    //        if (!pDrawCall.mShininess.has_value())
    //            pDrawCall.mShininess = 64.f;
    //        ImGui::SliderFloat("Shininess", &pDrawCall.mShininess.value(), 0.1f, 128.f);
    //        if (!pDrawCall.mTextureRepeatFactor.has_value())
    //            pDrawCall.mTextureRepeatFactor = 1.f;
    //        ImGui::SliderFloat("Texture repeat factor", &pDrawCall.mTextureRepeatFactor.value(), 1.f, 128.f);
    //    }
    //    default:
    //        break;
    //    };
    }
}