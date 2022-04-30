#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

#include "Renderer.hpp"
#include "OpenGLAPI.hpp"
#include "Logger.hpp"
#include "Timer.hpp"

#include "cmath"
#include "numeric"
#include "chrono"

Renderer::Renderer()
: mDrawCount(0)
, mTargetFPS(60)
, mTextureManager()
, mMeshManager(mTextureManager)
, mLightManager()
, mOpenGLAPI(new OpenGLAPI(mLightManager))
, mCamera(glm::vec3(0.0f, 1.7f, 7.0f), std::bind(&GraphicsAPI::setView, mOpenGLAPI, std::placeholders::_1), std::bind(&GraphicsAPI::setViewPosition, mOpenGLAPI, std::placeholders::_1))
, mRenderImGui(true)
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
	mMeshManager.ForEach([this](const auto &mesh) { mOpenGLAPI->initialiseMesh(mesh); }); // Depends on mShaders being initialised.
	mTextureManager.ForEach([this](const auto &texture) { mOpenGLAPI->initialiseTexture(texture); });

	lightPosition.mScale 		= glm::vec3(0.1f);
	lightPosition.mMesh 		= mMeshManager.getMeshID("3DCube");
	lightPosition.mColour		= glm::vec3(1.f);
	lightPosition.mDrawStyle 	= DrawStyle::UniformColour;

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
			DrawCall &drawCall = mDrawCalls.Create(ECS::CreateEntity());
			drawCall.mPosition = cubePositions[i];
			drawCall.mMesh = mMeshManager.getMeshID("3DCube");

			drawCall.mDrawStyle = DrawStyle::LightMap;
			drawCall.mDiffuseTextureID = mTextureManager.getTextureID("metalContainerDiffuse");
			drawCall.mSpecularTextureID = mTextureManager.getTextureID("metalContainerSpecular");
			drawCall.mShininess = 64.f;
		}
	}
	{ // Floor
		DrawCall &drawCall = mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mPosition;
		drawCall.mRotation.x = -90.f;
		drawCall.mScale = glm::vec3(25.f);
		drawCall.mMesh = mMeshManager.getMeshID("Quad");
		drawCall.mDrawStyle = DrawStyle::LightMap;
		drawCall.mDiffuseTextureID = mTextureManager.getTextureID("grassTile");
		drawCall.mSpecularTextureID = mTextureManager.getTextureID("black");
		drawCall.mShininess = 128.f;
		drawCall.mTextureRepeatFactor = 20.f;
	}
	{
		DrawCall &drawCall = mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mPosition = glm::vec3(-3.0f, 1.0f, 1.f);
		drawCall.mScale = glm::vec3(0.5f);
		drawCall.mMesh = mMeshManager.getMeshID("backpack");
		drawCall.mDrawStyle = DrawStyle::LightMap;
		drawCall.mDiffuseTextureID = mTextureManager.getTextureID("diffuse");
		drawCall.mSpecularTextureID = mTextureManager.getTextureID("specular");
		drawCall.mShininess = 64.f;
	}
	{
		DrawCall &drawCall = mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mPosition = glm::vec3(8.0f, 10.0f, 0.0f);
		drawCall.mRotation = glm::vec3(-10.0f, 230.0f, -15.0f);
		drawCall.mScale = glm::vec3(0.4f);
		drawCall.mMesh = mMeshManager.getMeshID("xian");
		drawCall.mDrawStyle = DrawStyle::LightMap;
		drawCall.mDiffuseTextureID = mTextureManager.getTextureID("Base_Color");
		drawCall.mSpecularTextureID = mTextureManager.getTextureID("black");
		drawCall.mShininess = 64.f;
	}

	{ // Billboard grass
		std::array<glm::vec3, 5> vegetation = {
			glm::vec3(-1.5f, 0.0f, -0.48f),
			glm::vec3(1.5f, 0.0f, 0.51f),
			glm::vec3(0.0f, 0.0f, 0.7f),
			glm::vec3(-0.3f, 0.0f, -2.3f),
			glm::vec3(0.5f, 0.0f, -0.6f)};

		for (const auto& position : vegetation)
		{
			DrawCall &drawCall = mDrawCalls.Create(ECS::CreateEntity());
			drawCall.mScale = glm::vec3(0.2f);
			drawCall.mPosition = position;
			drawCall.mPosition.y += drawCall.mScale.y;

			drawCall.mMesh = mMeshManager.getMeshID("Quad");
			drawCall.mDrawStyle = DrawStyle::Textured;
			drawCall.mTexture1 = mTextureManager.getTextureID("grassBillboard");
		}
	}

	{// Windows
		std::array<glm::vec3, 5> windowPositions = {
			glm::vec3(-1.5f, 0.0f, 1.48f),
			glm::vec3(1.5f, 0.0f, 1.51f),
			glm::vec3(0.0f, 0.0f, 1.7f),
			glm::vec3(-0.3f, 0.0f, 1.3f),
			glm::vec3(0.5f, 0.0f, 1.6f)};

		for (const auto& position : windowPositions)
		{
			DrawCall &drawCall = mDrawCalls.Create(ECS::CreateEntity());
			drawCall.mScale = glm::vec3(0.2f);
			drawCall.mPosition = position;
			drawCall.mPosition.y += drawCall.mScale.y;

			drawCall.mMesh = mMeshManager.getMeshID("Quad");
			drawCall.mDrawStyle = DrawStyle::Textured;
			drawCall.mTexture1 = mTextureManager.getTextureID("window");
		}
	}
}

Renderer::~Renderer()
{
	delete mOpenGLAPI;
}

void Renderer::onFrameStart(const std::chrono::microseconds& pTimeSinceLastDraw)
{
	mTimeSinceLastDraw 	= static_cast<float>(pTimeSinceLastDraw.count()) / 1000.0f; // Convert microseconds to milliseconds

	if (mUseRawPerformanceData)
		mCurrentFPS = 1.0f / (static_cast<float>((pTimeSinceLastDraw.count()) / 1000000.0f));
	else
		mCurrentFPS = (mDataSmoothingFactor * (1.0f / (static_cast<float>((pTimeSinceLastDraw.count()) / 1000000.0f)))) + (1.0f - mDataSmoothingFactor) * mCurrentFPS;

	mOpenGLAPI->onFrameStart();

	{ // Setup lights in GraphicsAPI
		mLightManager.getPointLights().ForEach([this](const PointLight& pPointLight) { mOpenGLAPI->draw(pPointLight); });
		mLightManager.getDirectionalLights().ForEach([this](const DirectionalLight& pDirectionalLight) { mOpenGLAPI->draw(pDirectionalLight); });
		mLightManager.getSpotlightsLights().ForEach([this](const SpotLight& pSpotLight) { mOpenGLAPI->draw(pSpotLight); });
	}
}

void Renderer::draw(const std::chrono::microseconds& pTimeSinceLastDraw)
{
	Stopwatch stopwatch;

	onFrameStart(pTimeSinceLastDraw);
	{ // Draw all meshes via DrawCalls
		mDrawCalls.ForEach([&](const DrawCall &pDrawCall)
		{
			mOpenGLAPI->draw(pDrawCall);
		});

		if (mLightManager.mRenderLightPositions)
		{
			mLightManager.getPointLights().ForEach([&](const PointLight &pPointLight)
			{
				lightPosition.mPosition = pPointLight.mPosition;
				lightPosition.mColour	= pPointLight.mColour;
				mOpenGLAPI->draw(lightPosition);
			});
		}
	}
	postDraw();

	mDrawCount++;
	mDrawTimeTakenMS = stopwatch.getTime<std::milli, float>();
}

void Renderer::postDraw()
{
	renderImGui();
	mOpenGLAPI->postDraw(); // Swaps the buffers, must be called after draw
}

void Renderer::renderImGui()
{
	// Render all ImGui from here.
	// Regardless of mRenderImGui, we call newImGuiFrame() and renderImGuiFrame() to allow showing performance window.
	Stopwatch stopWatch;

	mOpenGLAPI->newImGuiFrame();

	if (mRenderImGui)
	{
		if (ImGui::Begin("Render options", nullptr))
		{
			ImGui::Checkbox("Render light positions", &mLightManager.mRenderLightPositions);
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

		if (ImGui::Begin("Entity draw options"))
		{
			size_t count = 0;

			mDrawCalls.ModifyForEach([&](DrawCall& pDrawCall)
									 {
			count++;
			const std::string title = "Draw call option " + std::to_string(count);

			if(ImGui::TreeNode(title.c_str()))
			{
				ImGui::SliderFloat3("Position", &pDrawCall.mPosition.x, -50.f, 50.f);
				ImGui::SliderFloat3("Rotation", &pDrawCall.mRotation.x, -360.f, 360.f);
				ImGui::SliderFloat3("Scale", &pDrawCall.mScale.x, 0.1f, 10.f);

				{ // Draw mode selection
					if (ImGui::BeginCombo("Draw Mode", convert(pDrawCall.mDrawMode).c_str(), ImGuiComboFlags()))
					{
						for (size_t i = 0; i < drawModes.size(); i++)
						{
							if (ImGui::Selectable(drawModes[i].c_str()))
								pDrawCall.mDrawMode = static_cast<DrawMode>(i);
						}
						ImGui::EndCombo();
					}
				}

				{ // Draw style selection
					if (ImGui::BeginCombo("Draw Style", convert(pDrawCall.mDrawStyle).c_str(), ImGuiComboFlags()))
					{
						for (size_t i = 0; i < drawStyles.size(); i++)
						{
							if (ImGui::Selectable(drawStyles[i].c_str()))
								pDrawCall.mDrawStyle = static_cast<DrawStyle>(i);
						}
						ImGui::EndCombo();
					}
				}

				ImGui::Separator();

				switch (pDrawCall.mDrawStyle)
				{
				case DrawStyle::Textured:
				{
					{// Texture 1
						const std::string currentTexture = pDrawCall.mTexture1.has_value() ? mTextureManager.getTextureName(pDrawCall.mTexture1.value()) : "Empty";
						if (ImGui::BeginCombo("Texture", currentTexture.c_str(), ImGuiComboFlags()))
						{
							mTextureManager.ForEach([&](const Texture &texture)
							{
								if (ImGui::Selectable(texture.mName.c_str()))
								{
									pDrawCall.mTexture1 = texture.getID();
								}
							});
							ImGui::EndCombo();
						}
					}
					if (pDrawCall.mTexture1.has_value())
					{ // Texture 2
						const std::string currentTexture = pDrawCall.mTexture2.has_value() ? mTextureManager.getTextureName(pDrawCall.mTexture2.value()) : "Empty";
						if (ImGui::BeginCombo("Texture 2", currentTexture.c_str(), ImGuiComboFlags()))
						{
							if	(pDrawCall.mTexture2.has_value())
								if (ImGui::Selectable("Empty"))
									pDrawCall.mTexture2 = std::nullopt;

							mTextureManager.ForEach([&](const Texture &texture)
							{
								if (ImGui::Selectable(texture.mName.c_str()))
								{
									pDrawCall.mTexture2 = texture.getID();
								}
							});
							ImGui::EndCombo();
						}
					}
					if (pDrawCall.mTexture1.has_value() && pDrawCall.mTexture2.has_value())
					{ // Only displayed if we have two texture slots set
						if (!pDrawCall.mMixFactor.has_value())
							pDrawCall.mMixFactor = 0.5f;

						ImGui::SliderFloat("Texture mix factor", &pDrawCall.mMixFactor.value(), 0.f, 1.f);
					}
				}
				break;
				case DrawStyle::UniformColour:
				{
					if (!pDrawCall.mColour.has_value())
						pDrawCall.mColour = glm::vec3(1.f, 1.f, 1.f);

					ImGui::ColorEdit3("Colour",  &pDrawCall.mColour.value().x);
				}
				break;
				case DrawStyle::LightMap:
				{
					ImGui::Text("Available texture slots");
					{
						const std::string currentTexture = pDrawCall.mDiffuseTextureID.has_value() ? mTextureManager.getTextureName(pDrawCall.mDiffuseTextureID.value()) : "No texture set";
						if (ImGui::BeginCombo("Diffuse", currentTexture.c_str(), ImGuiComboFlags()))
						{
							mTextureManager.ForEach([&](const Texture &texture)
							{
								if (ImGui::Selectable(texture.mName.c_str()))
									pDrawCall.mDiffuseTextureID = texture.getID();
							});
							ImGui::EndCombo();
						}
					}
					{
						const std::string currentTexture = pDrawCall.mSpecularTextureID.has_value() ? mTextureManager.getTextureName(pDrawCall.mSpecularTextureID.value()) : "No texture set";
						if (ImGui::BeginCombo("Specular", currentTexture.c_str(), ImGuiComboFlags()))
						{
							mTextureManager.ForEach([&](const Texture &texture)
							{
								if (ImGui::Selectable(texture.mName.c_str()))
									pDrawCall.mSpecularTextureID = texture.getID();
							});
							ImGui::EndCombo();
						}
					}
					if (!pDrawCall.mShininess.has_value())
						pDrawCall.mShininess = 64.f;
					ImGui::SliderFloat("Shininess", &pDrawCall.mShininess.value(), 0.1f, 128.f);

					if (!pDrawCall.mTextureRepeatFactor.has_value())
						pDrawCall.mTextureRepeatFactor = 1.f;
					ImGui::SliderFloat("Texture repeat factor", &pDrawCall.mTextureRepeatFactor.value(), 1.f, 128.f);
				}
				default:
					break;
				}
				ImGui::TreePop();
			} });
		}
		ImGui::End();

		ImGui::ShowDemoWindow();
		ImGui::ShowMetricsWindow();

		mLightManager.renderImGui();
		mOpenGLAPI->renderImGui();
	}

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