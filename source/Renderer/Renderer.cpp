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

	// Draw all the render information components
	for (size_t i = 0; i < mMeshComponents.GetCount(); ++i)
	{
		ZEPHYR_ASSERT(mMeshComponents[i].mDrawMode != Mesh::DrawMode::Unknown, "Attempting to draw a mesh that was un-initialised using the graphics context")
		mGraphicsContext->draw(mMeshComponents[i]);
	}

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
		drawFrame();
	}

	LOG_INFO("Graphics context has requested closure exiting render loop.");
}