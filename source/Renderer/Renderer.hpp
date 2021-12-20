#pragma once

//#include "ComponentManager.hpp"

class Context;

class Renderer
{
public:
	bool initialise();
	void draw();
	int drawCount = 0;

private:
	void prepareFrame();
	Context *mGraphicsContext = nullptr;
	//ECS::ComponentManager<Mesh> mMeshComponents;
};