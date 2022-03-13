#include "LightManager.hpp"

#include "Entity.hpp"

#include "imgui.h"

LightManager::LightManager()
{
	PointLight& pointLight = mPointLights.Create(ECS::CreateEntity());
	pointLight.mPosition = glm::vec3(2.5f, 2.5f, 1.3f);
}

void LightManager::outputImGui()
{

	if (ImGui::Begin("Light options"))
	{
		size_t count = 0;

		mPointLights.ModifyForEach([&](PointLight& pPointLight)
		{
			count++;
			const std::string title = "Pointlight " + std::to_string(count);

			if(ImGui::TreeNode(title.c_str()))
			{
				ImGui::SliderFloat3("Position", &pPointLight.mPosition.x, -10.f, 10.f);
                ImGui::ColorEdit3("Colour",   &pPointLight.mColour.x);
				//ImGui::ColorEdit3("Colour",   &pPointLight.mColour.x);
				ImGui::SliderFloat("Ambient intensity",   &pPointLight.mAmbientIntensity, 0.f, 1.f);
				ImGui::SliderFloat("Diffuse intensity",   &pPointLight.mDiffuseIntensity, 0.f, 1.f);
				ImGui::SliderFloat("Specular intensity",  &pPointLight.mSpecularIntensity, 0.f, 1.f);

				ImGui::TreePop();
			}
		});
	}
	ImGui::End();
}