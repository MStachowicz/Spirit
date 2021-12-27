#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

#include "Renderer.hpp"
#include "Logger.hpp"
#include "OpenGLContext.hpp"

Renderer::~Renderer()
{
	delete mGraphicsContext;
}

bool Renderer::initialise()
{
	mGraphicsContext = new OpenGLContext();
	if (!mGraphicsContext->initialise())
	{
		LOG_CRITICAL("Failed to initalise the graphics context for Renderer to use.");
		return false;
	}

	return true;
}

void Renderer::onFrameStart()
{
	mGraphicsContext->onFrameStart();
}

void Renderer::draw()
{
	{
		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(0.25f);
		drawCall.mPosition 			= glm::vec3(-0.75f, 0.75f, 0.0f);
		drawCall.mMesh 				= mGraphicsContext->getMeshID("Square");
		drawCall.mTexture			= mGraphicsContext->getTextureID("tiles.png");
		mGraphicsContext->pushDrawCall(drawCall);
	}
		{
		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(0.25f);
		drawCall.mPosition 			= glm::vec3(0.0f, 0.75f, 0.0f);
		drawCall.mMesh 				= mGraphicsContext->getMeshID("Square");
		drawCall.mDrawMode			= DrawCall::DrawMode::Wireframe;
		mGraphicsContext->pushDrawCall(drawCall);
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
		drawCall.mMesh 				= mGraphicsContext->getMeshID("Cube");
		drawCall.mTexture 			= mGraphicsContext->getTextureID("woodenContainer.png");
		mGraphicsContext->pushDrawCall(drawCall);
	}
	{
		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(0.25f);
		drawCall.mPosition 			= glm::vec3(-0.75f, -0.75f, 0.0f);
		drawCall.mMesh 				= mGraphicsContext->getMeshID("Triangle");
		drawCall.mTexture			= mGraphicsContext->getTextureID("tiles.png");
		mGraphicsContext->pushDrawCall(drawCall);
	}
	{
		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(0.25f, 0.5f, 0.25f);
		drawCall.mPosition 			= glm::vec3(0.0f, -0.75f, 0.0f);
		drawCall.mMesh 				= mGraphicsContext->getMeshID("Triangle");
		mGraphicsContext->pushDrawCall(drawCall);
	}
	{
		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(0.25f);
		drawCall.mPosition 			= glm::vec3(0.75f, -0.75f, 0.0f);
		drawCall.mMesh 				= mGraphicsContext->getMeshID("Triangle");
		mGraphicsContext->pushDrawCall(drawCall);
	}

	mGraphicsContext->draw();
	drawCount++;
}