#pragma once

//#include "ComponentManager.hpp"

class Context;

class Renderer
{
public:
	bool initialise();
	void prepareFrame();
	void drawFrame();
	void drawLoop();

	private:
		Context *mGraphicsContext;
		//ECS::ComponentManager<Mesh> mMeshComponents;
};