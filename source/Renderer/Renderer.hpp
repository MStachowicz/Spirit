#pragma once

#include "LightManager.hpp"
#include "TextureManager.hpp"
#include "MeshManager.hpp"
#include "Camera.hpp"

#include "DrawCall.hpp"

#include "ComponentManager.hpp"

class GraphicsAPI;

// Submits DrawCalls to it's GraphicsAPI which itself implements the rendering pipeline being used.
class Renderer
{
public:
	Renderer();
	~Renderer();

	void onFrameStart(const std::chrono::microseconds& pTimeSinceLastDraw);
	void draw(const std::chrono::microseconds& pTimeSinceLastDraw);
	void postDraw();

	int mDrawCount;
	int mTargetFPS; // Independently of physics, the number of frames the renderer will aim to draw per second.
	Camera& getCamera() { return mCamera; }
private:
	// Order of initialisation is important here:
	float mCurrentFPS; // The FPS in the current frame.
	float mCurrentFPSSmoothingFactor; // The factor to smooth the current FPS with. 0 = quickly discard old value, 1= keep effect of old values longer
	float mCurrentFPSSmoothed; // The FPS averaged over previous frames to provide stable value to represent current FPS.
	int mFPSSampleSize; // The number of frames used to graph the FPS and calculate the average.
	float mAverageFPS; // The average fps over the last mFPSSampleSize frames.
	std::vector<float> mFPSTimes; // Holds the last mFPSSampleSize frame times.

	TextureManager mTextureManager;
	MeshManager mMeshManager;
	LightManager mLightManager;
	GraphicsAPI *mOpenGLAPI;
	Camera mCamera;

	DrawCall lightPosition;
	ECS::ComponentManager<DrawCall> mDrawCalls;
};