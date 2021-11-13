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

void Renderer::drawLoop()
{
	if(!mGraphicsContext)
	{
		LOG_ERROR("Calling draw with no context set. Initialise the graphics API/Window first");
		return;
	}

	while (!mGraphicsContext->isClosing())
	{
		mGraphicsContext->pollEvents();
		
		mGraphicsContext->clearWindow();

		// call drawing functions here

		mGraphicsContext->swapBuffers();
	}

	LOG_INFO("Graphics context has requested closure exiting.");
}