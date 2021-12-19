#include "Renderer.hpp"
#include "Logger.hpp"
#include "OpenGLContext.hpp"

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

void Renderer::prepareFrame()
{
	mGraphicsContext->clearWindow();
	mGraphicsContext->newImGuiFrame();
}

void Renderer::draw()
{
	prepareFrame();

	mGraphicsContext->renderImGuiFrame();

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
		DrawCall drawCall;
		drawCall.mScale 			= glm::vec3(0.25f);
		drawCall.mPosition 			= glm::vec3(0.75f, 0.75f, 0.0f);
		//drawCall.mRotation 			= glm::vec3(0.0f, 0.0f, glfwGetTime());
		drawCall.mMesh 				= mGraphicsContext->getMeshID("Square");
		drawCall.mTexture 	= mGraphicsContext->getTextureID("woodenContainer.png");
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
	mGraphicsContext->swapBuffers();
	drawCount++;
}