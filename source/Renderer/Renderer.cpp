#include "Renderer.hpp"
#include "OpenGLAPI.hpp"
#include "Logger.hpp"
#include "Timer.hpp"

#include "EntityManager.hpp"
#include "EventDispatcher.hpp"

#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

#include <cmath>
#include <numeric>
#include <chrono>

Renderer::Renderer(ECS::EntityManager& pEntityManager)
: mDrawCount(0)
, mTargetFPS(60)
, mTextureManager()
, mMeshManager(mTextureManager)
, mOpenGLAPI(new OpenGLAPI())
, mCamera(glm::vec3(0.0f, 1.7f, 7.0f), std::bind(&GraphicsAPI::setView, mOpenGLAPI, std::placeholders::_1), std::bind(&GraphicsAPI::setViewPosition, mOpenGLAPI, std::placeholders::_1))
, mEntityManager(pEntityManager)
, mRenderImGui(true)
, mRenderLightPositions(true)
, mShowFPSPlot(false)
, mUseRawPerformanceData(false)
, mDataSmoothingFactor(0.1f)
, mFPSSampleSize(120)
, mAverageFPS(0)
, mCurrentFPS(0)
, mTimeSinceLastDraw(0)
, mImGuiRenderTimeTakenMS(0)
, mDrawTimeTakenMS(0)
{
	mMeshManager.ForEach([this](const auto& mesh) { mOpenGLAPI->initialiseMesh(mesh); }); // Depends on mShaders being initialised.
	mTextureManager.ForEach([this](const auto& texture) { mOpenGLAPI->initialiseTexture(texture); });
	mTextureManager.ForEachCubeMap([this](const auto& cubeMap) { mOpenGLAPI->initialiseCubeMap(cubeMap); });

	lightPosition.mMesh.mID 		= mMeshManager.getMeshID("3DCube");
	lightPosition.mMesh.mColour		= glm::vec3(1.f);
	lightPosition.mMesh.mDrawStyle 	= Data::DrawStyle::UniformColour;

	const float floorSize = 25.f;
	const size_t grassCount = 10000;

	{ // Cubes
		std::array<glm::vec3, 10> cubePositions = {
			glm::vec3(3.0f,   0.5f,  0.0f),
			glm::vec3(-1.3f,  0.5f, -1.5f),
			glm::vec3(1.5f,	  0.5f, -2.5f),
			glm::vec3(-1.5f,  0.5f, -2.5f),
			glm::vec3(2.4f,   0.5f, -3.5f),
			glm::vec3(1.5f,   0.5f, -5.5f),
			glm::vec3(-1.7f,  0.5f, -7.5f),
			glm::vec3(1.3f,   0.5f, -8.5f),
			glm::vec3(-3.8f,  0.5f, -12.3f),
			glm::vec3(2.0f,   0.5f, -15.0f)};
		for (size_t i = 0; i < cubePositions.size(); i++)
		{
			auto& entity = mEntityManager.CreateEntity();

			Data::Transform transform;
			transform.mPosition = cubePositions[i];
			mEntityManager.mTransforms.Add(entity, transform);

			Data::MeshDraw mesh;
			mesh.mID = mMeshManager.getMeshID("3DCube");
			mesh.mName = "3DCube";
			mesh.mDrawStyle = Data::DrawStyle::LightMap;
			mesh.mDiffuseTextureID = mTextureManager.getTextureID("metalContainerDiffuse");
			mesh.mSpecularTextureID = mTextureManager.getTextureID("metalContainerSpecular");
			mesh.mShininess = 64.f;
			mEntityManager.mMeshes.Add(entity, mesh);
		}
	}
	{ // Floor
		auto& entity = mEntityManager.CreateEntity();

		Data::Transform transform;
		transform.mPosition;
		transform.mRotation.x = -90.f;
		transform.mScale = glm::vec3(floorSize);
		mEntityManager.mTransforms.Add(entity, transform);

		Data::MeshDraw mesh;
		mesh.mID = mMeshManager.getMeshID("Quad");
		mesh.mDrawStyle = Data::DrawStyle::LightMap;
		mesh.mDiffuseTextureID = mTextureManager.getTextureID("grassTile");
		mesh.mSpecularTextureID = mTextureManager.getTextureID("black");
		mesh.mShininess = 128.f;
		mesh.mTextureRepeatFactor = 20.f;
		mEntityManager.mMeshes.Add(entity, mesh);
	}
	{ // Backpack
		auto& entity = mEntityManager.CreateEntity();

		Data::Transform transform;
		transform.mPosition = glm::vec3(-3.0f, 1.0f, 1.f);
		transform.mScale = glm::vec3(0.5f);
		mEntityManager.mTransforms.Add(entity, transform);

		Data::MeshDraw mesh;
		mesh.mID = mMeshManager.getMeshID("backpack");
		mesh.mDrawStyle = Data::DrawStyle::LightMap;
		mesh.mDiffuseTextureID = mTextureManager.getTextureID("diffuse");
		mesh.mSpecularTextureID = mTextureManager.getTextureID("specular");
		mesh.mShininess = 64.f;
		mEntityManager.mMeshes.Add(entity, mesh);

	}
	{ // Xian
		auto& entity = mEntityManager.CreateEntity();

		Data::Transform transform;
		transform.mPosition = glm::vec3(8.0f, 10.0f, 0.0f);
		transform.mRotation = glm::vec3(-10.0f, 230.0f, -15.0f);
		transform.mScale = glm::vec3(0.4f);
		mEntityManager.mTransforms.Add(entity, transform);

		Data::MeshDraw mesh;
		mesh.mID = mMeshManager.getMeshID("xian");
		mesh.mDrawStyle = Data::DrawStyle::LightMap;
		mesh.mDiffuseTextureID = mTextureManager.getTextureID("Base_Color");
		mesh.mSpecularTextureID = mTextureManager.getTextureID("black");
		mesh.mShininess = 64.f;
		mEntityManager.mMeshes.Add(entity, mesh);
	}
	{ // Billboard grass
		std::array<glm::vec3, grassCount> grassPositions;
		{
			std::array<float, grassCount> randomX;
			util::fillRandomNumbers(-floorSize, floorSize, randomX);
			std::array<float, grassCount> randomZ;
			util::fillRandomNumbers(-floorSize, floorSize, randomZ);
			for (size_t i = 0; i < grassCount; i++)
				grassPositions[i] = glm::vec3(randomX[i], 0.f, randomZ[i]);
		}

		for (const auto& position : grassPositions)
		{
			auto& entity = mEntityManager.CreateEntity();

			Data::Transform transform;
			transform.mScale = glm::vec3(0.2f);
			transform.mPosition = position;
			transform.mPosition.y += transform.mScale.y;
			mEntityManager.mTransforms.Add(entity, transform);

			Data::MeshDraw mesh;
			mesh.mID = mMeshManager.getMeshID("Quad");
			mesh.mDrawStyle = Data::DrawStyle::Textured;
			mesh.mTexture1 = mTextureManager.getTextureID("grassBillboard");
			mEntityManager.mMeshes.Add(entity, mesh);
		}
	}
	{ // Windows
		std::array<glm::vec3, 5> windowPositions = {
			glm::vec3(-1.5f, 0.0f, 1.48f),
			glm::vec3(1.5f, 0.0f, 1.51f),
			glm::vec3(0.0f, 0.0f, 1.7f),
			glm::vec3(-0.3f, 0.0f, 1.3f),
			glm::vec3(0.5f, 0.0f, 1.6f)};

		for (const auto& position : windowPositions)
		{
			auto& entity = mEntityManager.CreateEntity();


			Data::Transform transform;
			transform.mScale = glm::vec3(0.2f);
			transform.mPosition = position;
			transform.mPosition.y += transform.mScale.y;
			mEntityManager.mTransforms.Add(entity, transform);

			Data::MeshDraw mesh;
			mesh.mID = mMeshManager.getMeshID("Quad");
			mesh.mDrawStyle = Data::DrawStyle::Textured;
			mesh.mTexture1 = mTextureManager.getTextureID("window");
			mEntityManager.mMeshes.Add(entity, mesh);
		}
	}

	{// Lights
		{ // Point light
			const std::array<glm::vec3, 4> pointLightPositions = {
				glm::vec3(0.7f, 1.7f, 2.0f),
				glm::vec3(0.0f, 1.0f, -3.0f),
				glm::vec3(2.3f, 3.3f, -4.0f),
				glm::vec3(-4.0f, 2.0f, -12.0f)};

			for (const auto& position : pointLightPositions)
			{
				Data::PointLight pointLight;
				pointLight.mPosition = position;
				mEntityManager.mPointLights.Add(mEntityManager.CreateEntity(), pointLight);
			}
		}

		{ // Directional light
			Data::DirectionalLight directionalLight;
			directionalLight.mDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
			mEntityManager.mDirectionalLights.Add(mEntityManager.CreateEntity(), directionalLight);
		}
		// Spotlight
		mEntityManager.mSpotLights.Add(mEntityManager.CreateEntity(), {});
	}

	mEntityManager.ForEach([this](const ECS::Entity &pEntity){ parseEntity(pEntity); });
	mEntityManager.mTransforms.mChangedComponentEvent.Subscribe(std::bind(&Renderer::onTransformComponentChange, this, std::placeholders::_1, std::placeholders::_2));
}

Renderer::~Renderer()
{
	delete mOpenGLAPI;
}

void Renderer::onTransformComponentChange(const ECS::Entity& pEntity, const Data::Transform& pTransform)
{
	// Find the DrawCall containing pEntity Transform data and update the model matrix for it.
	for (auto& drawCall : mDrawCalls)
	{
		auto it = drawCall.mEntityModelIndexLookup.find(pEntity.mID);
		if (it != drawCall.mEntityModelIndexLookup.end())
		{
			drawCall.mModels[it->second] = util::GetModelMatrix(pTransform.mPosition, pTransform.mRotation, pTransform.mScale);
			return;
		}
	}
}

void Renderer::parseEntity(const ECS::Entity& pEntity)
{
	// Grab all the entities with meshes to draw then confirm they also have a transform component to use as a model matrix.
	if (const Data::MeshDraw* mesh = mEntityManager.mMeshes.GetComponent(pEntity))
	{
		if (const Data::Transform* transform = mEntityManager.mTransforms.GetComponent(pEntity))
		{
			DrawCall drawCall;
			drawCall.mMesh = *mesh;

			auto it = std::find_if(mDrawCalls.begin(), mDrawCalls.end(), [&drawCall](const DrawCall& entry)
			{
				return entry.mMesh.mID              == drawCall.mMesh.mID
				&& entry.mMesh.mDrawMode            == drawCall.mMesh.mDrawMode
				&& entry.mMesh.mDrawStyle           == drawCall.mMesh.mDrawStyle
				// Per DrawStyle values
				&& entry.mMesh.mTexture1            == drawCall.mMesh.mTexture1
				&& entry.mMesh.mTexture2            == drawCall.mMesh.mTexture2
				&& entry.mMesh.mMixFactor           == drawCall.mMesh.mMixFactor
				&& entry.mMesh.mColour              == drawCall.mMesh.mColour
				&& entry.mMesh.mDiffuseTextureID    == drawCall.mMesh.mDiffuseTextureID
				&& entry.mMesh.mSpecularTextureID   == drawCall.mMesh.mSpecularTextureID
				&& entry.mMesh.mShininess           == drawCall.mMesh.mShininess
				&& entry.mMesh.mTextureRepeatFactor == drawCall.mMesh.mTextureRepeatFactor;
			});

			if (it == mDrawCalls.end())
			{
				drawCall.mEntityModelIndexLookup[pEntity.mID] = drawCall.mModels.size();
				drawCall.mModels.push_back(util::GetModelMatrix(transform->mPosition, transform->mRotation, transform->mScale));
				mDrawCalls.push_back(drawCall);
			}
			else
			{
				it->mEntityModelIndexLookup[pEntity.mID] = it->mModels.size();
				it->mModels.push_back(util::GetModelMatrix(transform->mPosition, transform->mRotation, transform->mScale));
			}
		}
	}
}


void Renderer::onFrameStart(const std::chrono::microseconds& pTimeSinceLastDraw)
{
	mTimeSinceLastDraw 	= static_cast<float>(pTimeSinceLastDraw.count()) / 1000.0f; // Convert microseconds to milliseconds

	if (mUseRawPerformanceData)
		mCurrentFPS = 1.0f / (static_cast<float>((pTimeSinceLastDraw.count()) / 1000000.0f));
	else
		mCurrentFPS = (mDataSmoothingFactor * (1.0f / (static_cast<float>((pTimeSinceLastDraw.count()) / 1000000.0f)))) + (1.0f - mDataSmoothingFactor) * mCurrentFPS;

	mOpenGLAPI->preDraw();

	{ // Setup lights in GraphicsAPI
		mEntityManager.mPointLights.ForEach([this](const Data::PointLight& pPointLight) { mOpenGLAPI->draw(pPointLight); });
		mEntityManager.mDirectionalLights.ForEach([this](const Data::DirectionalLight& pDirectionalLight) { mOpenGLAPI->draw(pDirectionalLight); });
		mEntityManager.mSpotLights.ForEach([this](const Data::SpotLight& pSpotLight) { mOpenGLAPI->draw(pSpotLight); });
	}
}

void Renderer::draw(const std::chrono::microseconds& pTimeSinceLastDraw)
{
	Stopwatch stopwatch;

	onFrameStart(pTimeSinceLastDraw);
	{ // Draw all meshes via DrawCalls

		for (const auto& drawCall : mDrawCalls)
			if(!drawCall.mModels.empty())
				mOpenGLAPI->draw(drawCall);

		if (mRenderLightPositions)
		{
			lightPosition.mModels.clear();
			mEntityManager.mPointLights.ForEach([&](const Data::PointLight& pPointLight)
			{
				lightPosition.mMesh.mColour = pPointLight.mColour;
				lightPosition.mModels.push_back(util::GetModelMatrix(pPointLight.mPosition, {}, glm::vec3(0.1f)));
			});

			mOpenGLAPI->draw(lightPosition);
		}
	}
	postDraw();

	mDrawCount++;
	mDrawTimeTakenMS = stopwatch.getTime<std::milli, float>();
}

void Renderer::postDraw()
{
	mOpenGLAPI->postDraw();
	renderImGui(); // Render ImGui after all GraphicsAPI draw calls are finished before endFrame
	mOpenGLAPI->endFrame();
}

void Renderer::renderImGui()
{
	// Render all ImGui from here.
	Stopwatch stopWatch;

	mOpenGLAPI->newImGuiFrame();
	if (mRenderImGui)
	{
		if (ImGui::Begin("Render options", nullptr))
		{
			ImGui::Checkbox("Render light positions", &mRenderLightPositions);
		}
		ImGui::End();

		if (ImGui::Begin("ImGui options"))
		{
			ImGuiIO &io = ImGui::GetIO();
			ImGui::Text("DisplaySize: %.fx%.f", io.DisplaySize.x, io.DisplaySize.y);
			ImGui::Text("MainViewport()->DpiScale: %.3f", ImGui::GetMainViewport()->DpiScale);
			ImGui::DragFloat("FontGlobalScale", &io.FontGlobalScale, 0.005f, 0.3f, 4.0f, "%.1f");
			ImGui::Checkbox("WantCaptureMouse", &io.WantCaptureMouse);

			if(ImGui::TreeNode("Style editor"))
			{
				ImGui::ShowStyleEditor();
				ImGui::TreePop();
			}

		}
		ImGui::End();

		mEntityManager.DrawImGui();
		ImGui::ShowDemoWindow();
		ImGui::ShowMetricsWindow();

		mOpenGLAPI->renderImGui();
	}

	// Regardless of mRenderImGui, we call newImGuiFrame() and renderImGuiFrame() to allow showing performance window.
	if (ImGui::Begin("Performance"))
	{
		// This is showing the last frame's RenderTimeTaken since the update has to happen after renderImGuiFrame below.
		// TODO: Add above comment as a help marker.
		ImGui::Text("ImGui render took: %.3fms", 	mImGuiRenderTimeTakenMS);
		ImGui::Text("Render took: %.3fms", 			mDrawTimeTakenMS);
		ImGui::Text("Frame time: %.3f ms", 			mTimeSinceLastDraw);

		ImGui::Separator();
		ImGui::Text("Target FPS:%d", mTargetFPS);
		ImGui::Text("FPS:");

		ImVec4 colour;
		if (mCurrentFPS >= mTargetFPS * 0.99f)
			colour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		else if (mCurrentFPS <= mTargetFPS * 0.5f)
			colour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		else
			colour = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
		ImGui::SameLine();
		ImGui::TextColored(colour, "%.0f\t", mCurrentFPS);
		ImGui::SameLine();
		ImGui::Checkbox("Show plot", &mShowFPSPlot);
		if (mShowFPSPlot)
		{
			plotFPSTimes();

			// When changing mFPSSampleSize we have to clear the excess FPS entries in the start of the vector.
			if (ImGui::SliderInt("FPS frame sample size", &mFPSSampleSize, 1, 1000))
				if (mFPSSampleSize < mFPSTimes.size())
					mFPSTimes.erase(mFPSTimes.begin(), mFPSTimes.end() - mFPSSampleSize); // O(n) mFPSTimes.erase linear with mFPSSampleSize
		}

		if(ImGui::TreeNode("Options"))
		{
			ImGui::Checkbox("Render ImGui", &mRenderImGui);
			ImGui::Checkbox("Use raw data", &mUseRawPerformanceData); // Whether we use smoothing for the incoming values of mCurrentFPS.
			if (!mUseRawPerformanceData)
			{
				ImGui::SameLine();
				ImGui::SliderFloat("FPS smoothing factor", &mDataSmoothingFactor, 0.f, 1.f);
			}

			ImGui::TreePop();
		}
	}
	ImGui::End();

	mOpenGLAPI->renderImGuiFrame();
	mImGuiRenderTimeTakenMS = stopWatch.getTime<std::milli, float>();
}

void Renderer::plotFPSTimes()
{
	// We keep a list of mFPSTimes sampling the mCurrentFPS at every Renderer::Draw.
	// mAverageFPS gives the average FPS in the mFPSSampleSize of mFPSTimes.
	// When mFPSTimes is full, clears the first entry to allow ring buffer style push_back for output using ImGui::PlotLines
	if (mFPSTimes.size() <= mFPSSampleSize)
		mFPSTimes.push_back(mCurrentFPS);
	else
	{
		mFPSTimes.erase(mFPSTimes.begin()); // O(n) mFPSTimes.erase linear with mFPSTimes.size()
		mFPSTimes.push_back(mCurrentFPS);
	}
	mAverageFPS = std::reduce(mFPSTimes.begin(), mFPSTimes.end()) / static_cast<float>(mFPSTimes.size()); // O(?) reduce faster than accumulate, per frame
	ImGui::PlotLines("", &mFPSTimes[0], static_cast<int>(mFPSTimes.size()), 0, ("Avg:" + std::to_string(std::round(mAverageFPS))).c_str(), 0.0f, mTargetFPS * 1.25f, ImVec2(ImGui::GetWindowWidth(), mTargetFPS * 1.25f));
}