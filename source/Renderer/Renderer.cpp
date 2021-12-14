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

	{ // TRIANGLE - COLOUR ATTRIBUTES
		auto entity = ECS::CreateEntity();
		auto &mesh = mMeshComponents.Create(entity);
		mesh.mVertices = {
			-0.95f, -0.95f, 0.0f,  // left
			-0.0f,  -0.95f, 0.0f,  // right
			-0.5f,   0.0f,  0.0f,  // top
		};
		mesh.mColours = {
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f};

		mGraphicsContext->setHandle(mesh);
	}
	{ // TRIANGLE - COLOUR ATTRIBUTES
		auto entity = ECS::CreateEntity();
		auto &mesh = mMeshComponents.Create(entity);
		mesh.mVertices = {
			0.0f,  -0.95f, 0.0f, // left
			0.95f, -0.95f, 0.0f, // right
			0.5f,   0.0f,  0.0f	 // top
		};
		mesh.mColours = {
			0.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f};
		mGraphicsContext->setHandle(mesh);
	}
	{ // SQUARE - COLOUR ATTRIBUTES - ELEMENT BUFFER OBJECT DRAWN (mIndices)
		auto entity = ECS::CreateEntity();
		auto &mesh = mMeshComponents.Create(entity);
		mesh.mVertices = {
			-0.05f, 0.95f, 0.0f,
			-0.05f, 0.0f,  0.0f,
			-0.95f, 0.0f,  0.0f,
			-0.95f, 0.95f, 0.0f,
		};
		mesh.mColours = {
			0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f};
		mesh.mIndices = {
			0, 1, 3, // first triangle
			1, 2, 3	 // second triangle
		};

		mGraphicsContext->setHandle(mesh);
	}
	{ // SQUARE - TEXTURE
		auto entity = ECS::CreateEntity();
		auto &mesh = mMeshComponents.Create(entity);
		mesh.mVertices = {
			0.1f,  0.95f, 0.0f,
			0.1f,  0.0f,  0.0f,
			0.95f, 0.0f,  0.0f,
			0.95f, 0.95f, 0.0f,
		};
		mesh.mColours = {
			0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f};
		mesh.mTextureCoordinates = {
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f,
			0.0f, 1.0f};
		mesh.mTextures.push_back("woodenContainer.png");
		mesh.mTextures.push_back("awesomeface.png");
		mesh.mIndices = {
			0, 1, 3, // first triangle
			1, 2, 3	 // second triangle
		};

		mGraphicsContext->setHandle(mesh);
	}

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