#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

#include "Renderer.hpp"
#include "Logger.hpp"
#include "OpenGLAPI.hpp"

Renderer::~Renderer()
{
	delete mOpenGLAPI;
}

void Renderer::initialise()
{
	mOpenGLAPI = new OpenGLAPI();
	ZEPHYR_ASSERT(mOpenGLAPI->initialise(), "Failed to initialise OpenGL graphics API.");
}

void Renderer::onFrameStart()
{
	mOpenGLAPI->onFrameStart();
}

void Renderer::draw()
{
	{
		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(0.25f);
		drawCall.mPosition 			= glm::vec3(-0.75f, 0.75f, 0.0f);
		drawCall.mMesh 				= mOpenGLAPI->getMeshID("Square");
		drawCall.mTexture			= mOpenGLAPI->getTextureID("tiles.png");
		mOpenGLAPI->pushDrawCall(drawCall);
	}
		{
		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(0.25f);
		drawCall.mPosition 			= glm::vec3(0.0f, 0.75f, 0.0f);
		drawCall.mMesh 				= mOpenGLAPI->getMeshID("Square");
		drawCall.mDrawMode			= DrawCall::DrawMode::Wireframe;
		mOpenGLAPI->pushDrawCall(drawCall);
	}
	{
		static float position[] = {1.0, 0.0, 0.0};
		static float rotation[] = {0.0, 0.0, 0.0};
		static float scale[] 	= {1.0, 0.5, 0.5};
		static float color[4] 	= {1.0f, 1.0f, 1.0f, 1.0f};

		if(ImGui::Begin("Container options"))
		{
			ImGui::SliderFloat3("Position"	, position, -1.f, 1.f);
			ImGui::SliderFloat3("Rotation"	, rotation, -90.f, 90.f);
			ImGui::SliderFloat3("Scale"		, scale, 0.1f, 1.5f);
			ImGui::ColorEdit3("color"		, color);
		}
		ImGui::End();

		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(scale[0], scale[1], scale[2]);
		drawCall.mPosition 			= glm::vec3(position[0], position[1], position[2]);
		drawCall.mRotation 			= glm::vec3(rotation[0], rotation[1], rotation[2]);
		drawCall.mMesh 				= mOpenGLAPI->getMeshID("Cube");
		drawCall.mTexture 			= mOpenGLAPI->getTextureID("woodenContainer.png");
		mOpenGLAPI->pushDrawCall(drawCall);
	}
	{
		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(0.25f);
		drawCall.mPosition 			= glm::vec3(-0.75f, -0.75f, 0.0f);
		drawCall.mMesh 				= mOpenGLAPI->getMeshID("Triangle");
		drawCall.mTexture			= mOpenGLAPI->getTextureID("tiles.png");
		mOpenGLAPI->pushDrawCall(drawCall);
	}
	{
		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(0.25f, 0.5f, 0.25f);
		drawCall.mPosition 			= glm::vec3(0.0f, -0.75f, 0.0f);
		drawCall.mMesh 				= mOpenGLAPI->getMeshID("Triangle");
		mOpenGLAPI->pushDrawCall(drawCall);
	}
	{
		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(0.25f);
		drawCall.mPosition 			= glm::vec3(0.75f, -0.75f, 0.0f);
		drawCall.mMesh 				= mOpenGLAPI->getMeshID("Triangle");
		mOpenGLAPI->pushDrawCall(drawCall);
	}

	mOpenGLAPI->draw();
	drawCount++;
}