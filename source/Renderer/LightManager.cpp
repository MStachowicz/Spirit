#include "LightManager.hpp"

#include "Entity.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"
#include "array"

LightManager::LightManager()
{
	{ // Point light
		const std::array<glm::vec3, 4> pointLightPositions = {
			glm::vec3(0.7f, 0.2f, 2.0f),
			glm::vec3(2.3f, -3.3f, -4.0f),
			glm::vec3(-4.0f, 2.0f, -12.0f),
			glm::vec3(0.0f, 0.0f, -3.0f)};

		for (const auto position : pointLightPositions)
		{
			PointLight &pointLight = mPointLights.Create(ECS::CreateEntity());
			pointLight.mPosition = position;
		}
	}

	{ // Directional light
		DirectionalLight &directionalLight  = mDirectionalLights.Create(ECS::CreateEntity());
		directionalLight.mDirection 		= glm::vec3(-0.2f, -1.0f, -0.3f);
	}

	{// Spotlight
		SpotLight& spotlight = mSpotLights.Create(ECS::CreateEntity());
	}
}

void LightManager::outputImGui()
{

	if (ImGui::Begin("Light options"))
	{
		{ // Directional lights
			size_t count = 0;
			mDirectionalLights.ModifyForEach([&](DirectionalLight& directionalLight)
			{
				count++;
				const std::string title = "Directional light " + std::to_string(count);

				if(ImGui::TreeNode(title.c_str()))
				{
					if(ImGui::SliderFloat3("Direction", 	  &directionalLight.mDirection.x, -1.f, 1.f))
						glm::normalize(directionalLight.mDirection);
        	        ImGui::ColorEdit3("Colour", 			  &directionalLight.mColour.x);
					ImGui::SliderFloat("Ambient intensity",   &directionalLight.mAmbientIntensity, 0.f, 1.f);
					ImGui::SliderFloat("Diffuse intensity",   &directionalLight.mDiffuseIntensity, 0.f, 1.f);
					ImGui::SliderFloat("Specular intensity",  &directionalLight.mSpecularIntensity, 0.f, 1.f);

					ImGui::TreePop();
				}
			});
		}
		{ // Point lights
			size_t count = 0;
			mPointLights.ModifyForEach([&](PointLight& pointLight)
			{
				count++;
				const std::string title = "Pointlight " + std::to_string(count);

				if(ImGui::TreeNode(title.c_str()))
				{
					ImGui::SliderFloat3("Position", 		  &pointLight.mPosition.x, -10.f, 10.f);
        	        ImGui::ColorEdit3("Colour",   			  &pointLight.mColour.x);
					ImGui::SliderFloat("Ambient intensity",   &pointLight.mAmbientIntensity, 	0.f, 1.f);
					ImGui::SliderFloat("Diffuse intensity",   &pointLight.mDiffuseIntensity, 	0.f, 1.f);
					ImGui::SliderFloat("Specular intensity",  &pointLight.mSpecularIntensity, 	0.f, 1.f);
					ImGui::SliderFloat("Constant",   		  &pointLight.mConstant, 			0.f, 1.f);
					ImGui::SliderFloat("Linear",  			  &pointLight.mLinear, 				0.f, 1.f);
					ImGui::SliderFloat("Quadratic",  		  &pointLight.mQuadratic, 			0.f, 1.f);
					ImGui::TreePop();
				}
			});
		}
		{ // Spotlights
			size_t count = 0;
			mSpotLights.ModifyForEach([&](SpotLight& spotLight)
			{
				count++;
				const std::string title = "Spotlight " + std::to_string(count);

				if(ImGui::TreeNode(title.c_str()))
				{
					ImGui::SliderFloat3("Position", 		  &spotLight.mPosition.x, -1.f, 1.f);
					if(ImGui::SliderFloat3("Direction", 	  &spotLight.mDirection.x, -1.f, 1.f))
						glm::normalize(spotLight.mDirection);
        	        ImGui::ColorEdit3("Colour", 			  &spotLight.mColour.x);
					ImGui::SliderFloat("Ambient intensity",   &spotLight.mAmbientIntensity, 	0.f, 1.f);
					ImGui::SliderFloat("Diffuse intensity",   &spotLight.mDiffuseIntensity, 	0.f, 1.f);
					ImGui::SliderFloat("Specular intensity",  &spotLight.mSpecularIntensity, 	0.f, 1.f);
					ImGui::SliderFloat("Constant",   		  &spotLight.mConstant, 			0.f, 1.f);
					ImGui::SliderFloat("Linear",  			  &spotLight.mLinear, 				0.f, 1.f);
					ImGui::SliderFloat("Quadratic",  		  &spotLight.mQuadratic, 			0.f, 1.f);
					ImGui::SliderFloat("Cutoff",  			  &spotLight.mCutOff, 				0.f, 1.f);
					ImGui::SliderFloat("Outer cutoff",  	  &spotLight.mOuterCutOff, 			0.f, 1.f);

					ImGui::TreePop();
				}
			});
		}
	}
	ImGui::End();
}