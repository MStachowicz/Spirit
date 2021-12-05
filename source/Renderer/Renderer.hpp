#pragma once

#include "ComponentManager.hpp"
#include "GraphicsContext/Context.hpp"

class Renderer
{
public:
	bool initialise();
	void prepareFrame();
	void drawFrame();
	void drawLoop();

	private:
		Context *mGraphicsContext;
		ECS::ComponentManager<Mesh> mMeshComponents;
};