#pragma once

class Context;

class Renderer
{
public:
	~Renderer();

	bool initialise();
	void draw();
	int drawCount = 0;

	void onFrameStart();
private:
	Context *mGraphicsContext = nullptr;
};