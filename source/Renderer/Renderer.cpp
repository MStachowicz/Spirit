#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

#include "Renderer.hpp"
#include "Logger.hpp"
#include "OpenGLAPI.hpp"

Renderer::Renderer()
: mMeshManager()
, mTextureManager()
, mLightManager()
, mOpenGLAPI(new OpenGLAPI(mMeshManager, mTextureManager, mLightManager))
, mCamera(glm::vec3(0.0f, 0.0f, 7.0f)
, std::bind(&GraphicsAPI::setView, mOpenGLAPI, std::placeholders::_1)
, std::bind(&GraphicsAPI::setViewPosition, mOpenGLAPI, std::placeholders::_1)
)
{
	lightPosition.mScale 		= glm::vec3(0.1f);
	lightPosition.mMesh 		= mMeshManager.getMeshID("3DCube");
	lightPosition.mDrawStyle 	= DrawStyle::UniformColour;

	{
		DrawCall& drawCall		= mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mScale 		= glm::vec3(0.25f);
		drawCall.mPosition 		= glm::vec3(-0.75f, 0.75f, 0.0f);
		drawCall.mMesh 			= mMeshManager.getMeshID("2DSquare");
		//drawCall.mTexture		= mOpenGLAPI->getTextureID("tiles");
	}
	{
		DrawCall& drawCall		= mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mScale 		= glm::vec3(0.25f);
		drawCall.mPosition 		= glm::vec3(0.0f, 0.75f, 0.0f);
		drawCall.mMesh 			= mMeshManager.getMeshID("2DSquare");
		drawCall.mDrawMode 		= DrawMode::Wireframe;
	}
	{
		DrawCall& drawCall 		= mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mPosition 		= glm::vec3(1.f, 0.f, 0.f);
		drawCall.mMesh 			= mMeshManager.getMeshID("3DCube");
		drawCall.mTexture 		= mTextureManager.getTextureID("marcy");
	}
	{
		DrawCall& drawCall 		= mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mScale 		= glm::vec3(0.25f);
		drawCall.mPosition 		= glm::vec3(-0.75f, -0.75f, 0.0f);
		drawCall.mMesh 			= mMeshManager.getMeshID("2DTriangle");
		//drawCall.mTexture	= mOpenGLAPI->getTextureID("tiles");
	}
	{
		DrawCall& drawCall 		= mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mScale 		= glm::vec3(0.25f, 0.5f, 0.25f);
		drawCall.mPosition 		= glm::vec3(0.0f, -0.75f, 0.0f);
		drawCall.mMesh 			= mMeshManager.getMeshID("2DTriangle");
	}
	{
		DrawCall& drawCall 		= mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mScale 		= glm::vec3(0.25f);
		drawCall.mPosition 		= glm::vec3(0.75f, -0.75f, 0.0f);
		drawCall.mMesh 			= mMeshManager.getMeshID("2DTriangle");
	}
}

Renderer::~Renderer()
{
	delete mOpenGLAPI;
}

void Renderer::onFrameStart()
{
	mOpenGLAPI->onFrameStart();

	if (ImGui::Begin("Render options"))
	{
		ImGui::Checkbox("Render light positions", &mLightManager.mRenderLightPositions);
	}
	ImGui::End();

	if (ImGui::Begin("Entity draw options"))
	{
		size_t count = 0;

		mDrawCalls.ModifyForEach([&](DrawCall& pDrawCall)
		{
			count++;
			const std::string title = "Draw call option " + std::to_string(count);

			if(ImGui::TreeNode(title.c_str()))
			{
				ImGui::SliderFloat3("Position", &pDrawCall.mPosition.x, -1.f, 1.f);
				ImGui::SliderFloat3("Rotation", &pDrawCall.mRotation.x, -90.f, 90.f);
				ImGui::SliderFloat3("Scale",  &pDrawCall.mScale.x, 0.1f, 1.5f);
				//ImGui::ColorEdit3("color",  &pDrawCall.mColor.x);

				{ // Texture selection
					const std::string currentTexture = pDrawCall.mTexture.has_value() ?
					mTextureManager.getTextureName(pDrawCall.mTexture.value()) : "No texture set";

					if (ImGui::BeginCombo("Texture", currentTexture.c_str(), ImGuiComboFlags()))
					{
						mTextureManager.ForEach([&](const Texture& texture)
						{
							if (ImGui::Selectable(texture.mName.c_str()))
							{
								pDrawCall.mTexture = texture.mID;
								pDrawCall.mDrawStyle = DrawStyle::Textured;
							}
						});
						ImGui::EndCombo();
					}
				}

				{ // Draw style selection
					if (ImGui::BeginCombo("Draw Style", convert(pDrawCall.mDrawStyle).c_str(), ImGuiComboFlags()))
					{
						for (size_t i = 0; i < drawStyles.size(); i++)
						{
							if (ImGui::Selectable(drawStyles[i].c_str()))
								pDrawCall.mDrawStyle = static_cast<DrawStyle>(i);
						}
						ImGui::EndCombo();
					}
				}

				{ // Draw mode selection
					if (ImGui::BeginCombo("Draw Mode", convert(pDrawCall.mDrawMode).c_str(), ImGuiComboFlags()))
					{
						for (size_t i = 0; i < drawModes.size(); i++)
						{
							if (ImGui::Selectable(drawModes[i].c_str()))
								pDrawCall.mDrawMode = static_cast<DrawMode>(i);
						}
						ImGui::EndCombo();
					}
				}

				ImGui::TreePop();
			}
		});
	}
	ImGui::End();
}

void Renderer::draw()
{
	mDrawCalls.ForEach([&](const DrawCall& pDrawCall)
	{
		mOpenGLAPI->draw(pDrawCall);
	});

	if (mLightManager.mRenderLightPositions)
	{
		mLightManager.getPointLights().ForEach([&](const PointLight &pPointLight)
		{
			lightPosition.mPosition = pPointLight.mPosition;
			mOpenGLAPI->draw(lightPosition);
		});
	}

	drawCount++;
}

void Renderer::postDraw()
{
	mOpenGLAPI->postDraw();
}