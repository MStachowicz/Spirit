#pragma once

class GraphicsAPI;

// Submits DrawCalls to it's GraphicsAPI which itself implements the rendering pipeline being used.
class Renderer
{
public:
	~Renderer();

	bool initialise();
	void draw();
	int drawCount = 0;

	void onFrameStart();
private:
	GraphicsAPI *mOpenGLAPI = nullptr;
};