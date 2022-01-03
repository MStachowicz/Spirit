#pragma once

#include "Camera.hpp"

class GraphicsAPI;

// Submits DrawCalls to it's GraphicsAPI which itself implements the rendering pipeline being used.
class Renderer
{
public:
	Renderer();
	~Renderer();

	void onFrameStart();
	void draw();
	int drawCount = 0;

	Camera& getCamera() { return mCamera; }

private:
	GraphicsAPI *mOpenGLAPI;
	Camera mCamera;
};