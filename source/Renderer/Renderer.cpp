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
	else
		return true;
}

void Renderer::prepareFrame()
{
	mGraphicsContext->pollEvents();
	mGraphicsContext->clearWindow();
	mGraphicsContext->newImGuiFrame();
}

void Renderer::drawFrame()
{
	mGraphicsContext->renderImGuiFrame();
	mGraphicsContext->swapBuffers();
}

void Renderer::drawLoop()
{
	if(!mGraphicsContext)
	{
		LOG_ERROR("Calling draw with no context set. Initialise the graphics API/Window first");
		return;
	}

	while (!mGraphicsContext->isClosing())
	{
		prepareFrame();
		// call drawing functions here
		drawFrame();
	}

	LOG_INFO("Graphics context has requested closure exiting render loop.");
}