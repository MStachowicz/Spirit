#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

#include "Renderer.hpp"
#include "Logger.hpp"
#include "OpenGLAPI.hpp"

Renderer::Renderer()
: mMeshManager()
, mOpenGLAPI(new OpenGLAPI(mMeshManager, mTextureManager))
, mCamera(glm::vec3(0.0f, 0.0f, 7.0f)
, std::bind(&GraphicsAPI::setView, mOpenGLAPI, std::placeholders::_1)
, std::bind(&GraphicsAPI::setViewPosition, mOpenGLAPI, std::placeholders::_1)
)
{
	{
		DrawCall& drawCall		= mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mScale 		= glm::vec3(0.25f);
		drawCall.mPosition 		= glm::vec3(-0.75f, 0.75f, 0.0f);
		drawCall.mMesh 			= mMeshManager.getMeshID("2DSquare");
		//drawCall.mTexture		= mOpenGLAPI->getTextureID("tiles.png");
	}
	{
		DrawCall& drawCall		= mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mScale 		= glm::vec3(0.25f);
		drawCall.mPosition 		= glm::vec3(0.0f, 0.75f, 0.0f);
		drawCall.mMesh 			= mMeshManager.getMeshID("2DSquare");
		drawCall.mDrawMode 		= DrawCall::DrawMode::Wireframe;
	}
	{
		DrawCall& drawCall 		= mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mPosition 		= glm::vec3(1.f, 0.f, 0.f);
		drawCall.mMesh 			= mMeshManager.getMeshID("3DCube");
		// drawCall.mTexture 			= mOpenGLAPI->getTextureID("woodenContainer.png");
	}
	{
		DrawCall& drawCall 		= mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mScale 		= glm::vec3(0.25f);
		drawCall.mPosition 		= glm::vec3(-0.75f, -0.75f, 0.0f);
		drawCall.mMesh 			= mMeshManager.getMeshID("2DTriangle");
		// drawCall.mTexture			= mOpenGLAPI->getTextureID("tiles.png");
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

	size_t count = 0;

	if (ImGui::Begin("Entity draw options"))
	{
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
				ImGui::TreePop();
			}
		});
	}
	ImGui::End();
}

void Renderer::draw()
{
	static float position[] = {1.0, 0.0, 0.0};
	static float rotation[] = {0.0, 0.0, 0.0};
	static float scale[] = {1.0, 0.5, 0.5};
	static float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};

	mDrawCalls.ForEach([&](const DrawCall &pDrawCall)
	{
		mOpenGLAPI->draw(pDrawCall); });

	drawCount++;
}

void Renderer::postDraw()
{
	mOpenGLAPI->postDraw();
}