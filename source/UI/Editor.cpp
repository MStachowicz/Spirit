#include "Editor.hpp"

// ECS
#include "Storage.hpp"

// SYSTEM
#include "CollisionSystem.hpp"
#include "MeshSystem.hpp"
#include "SceneSystem.hpp"
#include "TextureSystem.hpp"

// COMPONENT
#include "Camera.hpp"
#include "Collider.hpp"
#include "DirectionalLight.hpp"
#include "Label.hpp"
#include "Mesh.hpp"
#include "PointLight.hpp"
#include "RigidBody.hpp"
#include "SpotLight.hpp"
#include "Texture.hpp"
#include "Transform.hpp"

// GEOMETRY
#include "Geometry.hpp"

// OPENGL
#include "OpenGLRenderer.hpp"

// PLATFORM
#include "Core.hpp"
#include "InputDefinitions.hpp"

// UTILITY
#include "Logger.hpp"

// GLM
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// IMGUI
#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

// STD
#include <format>

namespace ImGui
{
    bool ComboContainer(const char* pLabel, const char* pCurrentValue, const std::vector<std::string>& pItems, size_t& outSelectedIndex)
    {
        if (pItems.empty())
            return false;

        bool result = false;
        if (ImGui::BeginCombo(pLabel, pCurrentValue))
        {
            for (size_t i = 0; i < pItems.size(); i++)
            {
                if (ImGui::Selectable(pItems[i].c_str()))
                {
                    outSelectedIndex = i;
                    result = true;
                }
            }
            ImGui::EndCombo();
        }
        return result;
    }

    // Given a pCurrent and a list of pOptions and their labels, creates an ImGui selectable dropdown and assigns the selected option to pCurrent.
    // Returns true if pCurrent has been assigned a new value.
    template <typename Type>
    bool ComboContainer(const char* pLabel, Type& pCurrent, const std::vector<std::pair<Type, const char*>>& pOptions)
    {
        if (pOptions.empty())
            return false;

        const auto it = std::find_if(pOptions.begin(), pOptions.end(), [&pCurrent](const auto& pElement) { return pCurrent == pElement.first; });
        ASSERT(it != pOptions.end(), "pCurrent not found in the list pOptions, pOptions should be a complete list of all types of Type.");

        bool result = false;

        if (ImGui::BeginCombo(pLabel, it->second))
        {
            for (const auto& selectable : pOptions)
            {
                auto& [type, label] = selectable;

                if (ImGui::Selectable(label))
                {
                    pCurrent = type;
                    result = true;
                }
            }
            ImGui::EndCombo();
        }
        return result;
    }

    void Text(const char* pLabel, const glm::vec3& pVec3)
    {
        ImGui::Text(std::format("{}: {}, {}, {}", pLabel, pVec3.x, pVec3.y, pVec3.z).c_str());
    }
    void Text(const char* pLabel, const glm::mat4& pMat4)
    {
        ImGui::Text(std::format("{}:\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]"
        , pLabel
        , pMat4[0][0], pMat4[0][1], pMat4[0][2], pMat4[0][3]
        , pMat4[1][0], pMat4[1][1], pMat4[1][2], pMat4[1][3]
        , pMat4[2][0], pMat4[2][1], pMat4[2][2], pMat4[2][3]
        , pMat4[3][0], pMat4[3][1], pMat4[3][2], pMat4[3][3]

        ).c_str());
    }

    // Console with an input and filter view for outputting application messages to the editor.
    // The input box supports commands making this a command-line-esque console.
    struct Console
    {
        struct Message
        {
            std::string m_message;
            glm::vec3 m_colour;

            Message(const std::string& p_message, const glm::vec3& p_colour = glm::vec3(1.f, 1.f, 1.f))
                : m_message{p_message}
                , m_colour{p_colour}
            {}
        };

    private:
        std::vector<std::string> m_commands; // Available commands in the input box
        std::vector<std::string> m_history;
        std::vector<Message> m_log_messages;
        ImGuiTextFilter m_filter;
        char m_input_buffer[256]; // Text box input
        bool m_auto_scroll;
        bool m_scroll_to_bottom;

    public:
        Console()
            : m_commands{"HELP" "HISTORY" "CLEAR" "CLASSIFY"}
            , m_history{}
            , m_log_messages{}
            , m_auto_scroll{true}
            , m_scroll_to_bottom{false}
        {
            memset(m_input_buffer, 0, sizeof(m_input_buffer));
            clear_log();
        }
        ~Console() = default;

        void clear_log()                       { m_log_messages.clear(); }
        void add_log(const Message& p_message) { m_log_messages.push_back(p_message); }

        void draw(const char* title, bool* p_open)
        {
            ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin(title, p_open))
            {
                ImGui::End();
                return;
            }

            // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
            // So e.g. IsItemHovered() will return true when hovering the title bar.
            // Here we create a context menu only available from the title bar.
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Close Console"))
                    *p_open = false;
                ImGui::EndPopup();
            }
            ImGui::TextWrapped("Enter 'HELP' for help.");

            ImGui::SameLine();
            bool copy_to_clipboard = ImGui::SmallButton("Copy");
            // static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); add_log("Spam %f", t); }
            ImGui::Separator();

            // Options menu
                if (ImGui::BeginPopup("Options"))
                {
                ImGui::Checkbox("Auto-scroll", &m_auto_scroll);
                    ImGui::EndPopup();
                }

            // Options, m_filter
                if (ImGui::Button("Options"))
                    ImGui::OpenPopup("Options");
            ImGui::SameLine();
            m_filter.Draw("m_filter (\"incl,-excl\") (\"error\")", 180);
            ImGui::Separator();

            // Reserve enough left-over height for 1 separator + 1 input text
            const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::Selectable("Clear")) clear_log();
                ImGui::EndPopup();
            }

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

            if (copy_to_clipboard)
                ImGui::LogToClipboard();

            for (int i = 0; i < m_log_messages.size(); i++)
            {
                const char* item = m_log_messages[i].m_message.c_str();
                if (!m_filter.PassFilter(item))
                    continue;

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(m_log_messages[i].m_colour.r, m_log_messages[i].m_colour.g, m_log_messages[i].m_colour.b, 1.0f));
                ImGui::TextUnformatted(m_log_messages[i].m_message.c_str());
                ImGui::PopStyleColor();
            }
            if (copy_to_clipboard)
                ImGui::LogFinish();

            if (m_scroll_to_bottom || (m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);
            m_scroll_to_bottom = false;

            ImGui::PopStyleVar();
            ImGui::EndChild();
            ImGui::Separator();

            // Command-line
            bool reclaim_focus = false;
            ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
            if (ImGui::InputText("Input", m_input_buffer, IM_ARRAYSIZE(m_input_buffer), input_text_flags, &TextEditCallbackStub, (void*)this))
            {
                char* s = m_input_buffer;
                char* str_end = s + strlen(s);

                while (str_end > s && str_end[-1] == ' ')
                    str_end--;
                *str_end = 0;

                if (s[0])
                    ExecCommand(s);

                reclaim_focus = true;
            }

            // Auto-focus on window apparition
            ImGui::SetItemDefaultFocus();
            if (reclaim_focus)
                ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

            ImGui::End();
        }
    private:
        void ExecCommand(const std::string& p_command)
        {
            add_log(std::format("# {}\n", p_command));
            m_history.push_back(p_command);

            // Process command
            if (p_command == "CLEAR")
            {
                clear_log();
            }
            else if (p_command == "HELP")
                {
                ImGui::TextWrapped("TAB key - completion\nUp/Down keys - command history");

                std::string m_commandsStr = "Available m_commands:\n";
                for (const auto& command : m_commands)
                    m_commandsStr += "\n- " + command;
                add_log(m_commandsStr);
                }
            else if (p_command == "HISTORY")
            {
                std::string historyStr = "Command history:\n";
                for (const auto& command : m_history)
                    historyStr += "\n- " + command;
                add_log(historyStr);
            }
            else
                add_log({std::format("Unknown command: '{}'\n", p_command), glm::vec3(1.f, 0.f, 0.f)});

            // On command input, we scroll to bottom even if m_auto_scroll==false
            m_scroll_to_bottom = true;
        }

        // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
        static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
        {
            Console* console = (Console*)data->UserData;
            return console->TextEditCallback(data);
        }

        int TextEditCallback(ImGuiInputTextCallbackData* data)
        {
            // add_log("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
            switch (data->EventFlag)
            {
                case ImGuiInputTextFlags_CallbackCompletion:
                {
                    if (data->BufTextLen <= 0)
                        return 0;

                    std::string input = "";
                    input.reserve(data->BufSize);
                    for (int i = 0; i < data->BufTextLen; i++)
                        input.push_back(data->Buf[i]);

                    // Build a list of candidates
                    std::vector<std::string> candidates;
                    for (const auto& command : m_commands)
                    {
                        if (input == command || command.starts_with(input))
                            candidates.push_back(command);
                    }

                    if (candidates.size() == 0) // No match
                        add_log(std::format("No match for \"{}\"", input));
                    else if (candidates.size() == 1)
            {
                        // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                        data->DeleteChars(0, data->BufSize);
                        //data->DeleteChars(data->CursorPos - data->BufTextLen, data->CursorPos + data->BufTextLen);

                        data->InsertChars(data->CursorPos, candidates[0].c_str());
                        data->InsertChars(data->CursorPos, " ");
                    }
                    else
                {
                        // Multiple matches. Complete as much as we can..
                        // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.

                        // List matches
                        std::string candidatesStr = "";
                        for (const auto& candidate : candidates)
                            candidatesStr += std::format("- {}\n", candidate);
                        add_log(candidatesStr);
                    }
                    break;
                }
                case ImGuiInputTextFlags_CallbackHistory:
                    {
                    }
                }
            return 0;
            }
    };


} // namespace ImGui

namespace UI
{
    static ImGui::Console console;

    Editor::Editor(System::TextureSystem& pTextureSystem, System::MeshSystem& pMeshSystem, System::SceneSystem& pSceneSystem, System::CollisionSystem& pCollisionSystem, OpenGL::OpenGLRenderer& pOpenGLRenderer)
        : mTextureSystem{pTextureSystem}
        , mMeshSystem{pMeshSystem}
        , mSceneSystem{pSceneSystem}
        , mCollisionSystem{pCollisionSystem}
        , mOpenGLRenderer{pOpenGLRenderer}
        , mSelectedEntities{}
        , mWindowsToDisplay{}
        , mDrawCount{0}
    {
        Platform::Core::mMouseButtonEvent.subscribe(this, &Editor::onMousePressed);

        auto writeTriangleConstructor = [](const Geometry::Triangle& pTriangle)
        {
            auto constructor = std::format("Geometry::Triangle(glm::vec3({}f, {}f, {}f), glm::vec3({}f, {}f, {}f), glm::vec3({}f, {}f, {}f))"
            , pTriangle.mPoint1.x, pTriangle.mPoint1.y, pTriangle.mPoint1.z
            , pTriangle.mPoint2.x, pTriangle.mPoint2.y, pTriangle.mPoint2.z
            , pTriangle.mPoint3.x, pTriangle.mPoint3.y, pTriangle.mPoint3.z);
            std::cout << constructor << std::endl;
        };
    }

    void Editor::onMousePressed(const Platform::MouseButton& pMouseButton, const Platform::Action& pAction)
    {
        if (!Platform::Core::UICapturingMouse() && !Platform::Core::getWindow().capturingMouse())
        {
            switch (pMouseButton)
            {
                case Platform::MouseButton::MOUSE_LEFT:
                {
                    if (pAction == Platform::Action::PRESS)
                    {
                        auto cursorRay          = mOpenGLRenderer.getCursorWorldRay();
                        auto entitiesUnderMouse = mCollisionSystem.getEntitiesAlongRay(cursorRay);

                        if (!entitiesUnderMouse.empty())
                        {
                            std::sort(entitiesUnderMouse.begin(), entitiesUnderMouse.end(),[](const auto& left, const auto& right) { return left.second < right.second; });
                            auto entityCollided = entitiesUnderMouse.front().first;

                            mSelectedEntities.push_back(entityCollided);
                            LOG("Entity{} has been selected", entityCollided.ID);
                            }

                            const auto mouseRayCylinder  = Geometry::Cylinder(mSceneSystem.getPrimaryCamera()->getPosition(), mSceneSystem.getPrimaryCamera()->getPosition() + (cursorRay.mDirection * 1000.f), 0.02f);
                            mOpenGLRenderer.mDebugOptions.mCylinders.push_back(mouseRayCylinder);
                    }
                    break;
                }
                case Platform::MouseButton::MOUSE_MIDDLE:
                {
                    mOpenGLRenderer.mDebugOptions.mCylinders.clear();
                    break;
                }
                case Platform::MouseButton::MOUSE_RIGHT:
                {
                    if (pAction == Platform::Action::PRESS)
                        Platform::Core::getWindow().setInputMode(Platform::Core::getWindow().capturingMouse() ? Platform::CursorMode::NORMAL : Platform::CursorMode::CAPTURED);
                    break;
                }
            }
        }
    }

    void Editor::draw()
    {
        Platform::Core::startImGuiFrame();

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Entity hierarchy", NULL, &mWindowsToDisplay.Entity);
                ImGui::MenuItem("Log", NULL, &mWindowsToDisplay.Log);

                if (ImGui::BeginMenu("Debug"))
                {
                    ImGui::MenuItem("Performance", NULL, &mWindowsToDisplay.Performance);
                    ImGui::MenuItem("Graphics", NULL, &mWindowsToDisplay.Graphics);
                    ImGui::MenuItem("Physics", NULL, &mWindowsToDisplay.Physics);

                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("ImGui"))
                {
                    ImGui::MenuItem("Demo", NULL, &mWindowsToDisplay.ImGuiDemo);
                    ImGui::MenuItem("Metrics/Debugger", NULL, &mWindowsToDisplay.ImGuiMetrics);
                    ImGui::MenuItem("Stack", NULL, &mWindowsToDisplay.ImGuiStack);
                    ImGui::MenuItem("About", NULL, &mWindowsToDisplay.ImGuiAbout);
                    ImGui::MenuItem("Style Editor", NULL, &mWindowsToDisplay.ImGuiStyleEditor);

                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (mWindowsToDisplay.Entity)           drawEntityTreeWindow();
        if (mWindowsToDisplay.Log)              drawLog();
        if (mWindowsToDisplay.Performance)      drawPerformanceWindow();
        if (mWindowsToDisplay.Graphics)         drawGraphicsWindow();
        if (mWindowsToDisplay.Physics)          drawPhysicsWindow();
        if (mWindowsToDisplay.ImGuiDemo)        ImGui::ShowDemoWindow(&mWindowsToDisplay.ImGuiDemo);
        if (mWindowsToDisplay.ImGuiMetrics)     ImGui::ShowMetricsWindow(&mWindowsToDisplay.ImGuiMetrics);
        if (mWindowsToDisplay.ImGuiStack)       ImGui::ShowStackToolWindow(&mWindowsToDisplay.ImGuiStack);
        if (mWindowsToDisplay.ImGuiAbout)       ImGui::ShowAboutWindow(&mWindowsToDisplay.ImGuiAbout);
        if (mWindowsToDisplay.ImGuiStyleEditor)
        {
            ImGui::Begin("Dear ImGui Style Editor", &mWindowsToDisplay.ImGuiStyleEditor);
            ImGui::ShowStyleEditor();
            ImGui::End();
        }

        Platform::Core::endImGuiFrame();
        mDrawCount++;
    }

    void Editor::drawEntityTreeWindow()
    {
        if (ImGui::Begin("Entities", &mWindowsToDisplay.Entity))
        {
            auto& availableTextures = mTextureSystem.mAvailableTextures;
            std::vector<std::string> availableTextureNames;
            for (const auto& path : availableTextures)
                availableTextureNames.push_back(path.stem().string());

            auto& availableModels = mMeshSystem.mAvailableModels;
            std::vector<std::string> availableModelNames;
            for (const auto& path : availableModels)
                availableModelNames.push_back(path.stem().string());

            auto& scene = mSceneSystem.getCurrentScene();
            scene.foreachEntity([&](ECS::Entity& pEntity)
            {
                std::string title = "Entity " + std::to_string(pEntity.ID);
                if (scene.hasComponents<Component::Label>(pEntity))
                {
                    auto label = scene.getComponentMutable<Component::Label&>(pEntity);
                    title = label.mName;
                }

                if (ImGui::TreeNode(title.c_str()))
                {
                    if (scene.hasComponents<Component::Transform>(pEntity))
                        scene.getComponentMutable<Component::Transform&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::Collider>(pEntity))
                        scene.getComponentMutable<Component::Collider&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::RigidBody>(pEntity))
                        scene.getComponentMutable<Component::RigidBody&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::DirectionalLight>(pEntity))
                        scene.getComponentMutable<Component::DirectionalLight&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::SpotLight>(pEntity))
                        scene.getComponentMutable<Component::SpotLight&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::PointLight>(pEntity))
                        scene.getComponentMutable<Component::PointLight&>(pEntity).DrawImGui();
                    if (scene.hasComponents<Component::Camera>(pEntity))
                    {
                        auto& camera = scene.getComponentMutable<Component::Camera>(pEntity);
                        if (ImGui::TreeNode("Camera"))
                        {
                            ImGui::Text("Position", camera.getPosition());
                            ImGui::Text("Look direction", camera.getForwardDirection());
                            ImGui::Text("View matrix", camera.getViewMatrix());
                            ImGui::TreePop();
                        }
                    }
                    if (scene.hasComponents<Component::Mesh>(pEntity))
                    {
                        auto& mesh = scene.getComponentMutable<Component::Mesh>(pEntity);
                        if (ImGui::TreeNode("Mesh"))
                        {
                            auto current = mesh.mModel->mFilePath.stem().string();
                            static size_t selected;
                            if (ImGui::ComboContainer("Mesh", current.c_str(), availableModelNames, selected))
                                mesh.mModel = mMeshSystem.getModel(availableModels[selected]);

                            ImGui::TreePop();
                        }
                    }
                    if (scene.hasComponents<Component::Texture>(pEntity))
                    {
                        if (ImGui::TreeNode("Texture"))
                        {
                            auto& textureComponent = scene.getComponentMutable<Component::Texture&>(pEntity);
                            const std::string currentDiffuse  = textureComponent.mDiffuse.has_value() ? textureComponent.mDiffuse.value()->mName : "None";
                            const std::string currentSpecular = textureComponent.mSpecular.has_value() ? textureComponent.mSpecular.value()->mName : "None";

                            static size_t selected;
                            if (ImGui::ComboContainer("Diffuse Texture", currentDiffuse.c_str(), availableTextureNames, selected))
                                textureComponent.mDiffuse = mTextureSystem.getTexture(availableTextures[selected]);
                            if (ImGui::ComboContainer("Specular Texture", currentSpecular.c_str(), availableTextureNames, selected))
                                textureComponent.mSpecular = mTextureSystem.getTexture(availableTextures[selected]);

                            ImGui::TreePop();
                        }
                    }

                    ImGui::Separator();
                    ImGui::Separator();
                    ImGui::TreePop();
                }
            });
        }
        ImGui::End();
    }

    void Editor::drawLog()
    {
        console.draw("Console", &mWindowsToDisplay.Log);
    }

    void Editor::drawGraphicsWindow()
    {
        if (ImGui::Begin("Graphics", &mWindowsToDisplay.Graphics))
        {
            auto& window         = Platform::Core::getWindow();
            auto [width, height] = window.size();

            ImGui::Text(("Viewport size: " + std::to_string(width) + "x" + std::to_string(height)).c_str());
            ImGui::Text(("Aspect ratio: " + std::to_string(window.aspectRatio())).c_str());
            ImGui::Text("View Position", mOpenGLRenderer.mViewInformation.mViewPosition);
            ImGui::SliderFloat("Field of view", &mOpenGLRenderer.mViewInformation.mFOV, 1.f, 120.f);
            ImGui::SliderFloat("Z near plane", &mOpenGLRenderer.mViewInformation.mZNearPlane, 0.001f, 15.f);
            ImGui::SliderFloat("Z far plane", &mOpenGLRenderer.mViewInformation.mZFarPlane, 15.f, 300.f);
            ImGui::Separator();

            if (ImGui::TreeNode("PostProcessing"))
            {
                ImGui::Checkbox("Invert", &mOpenGLRenderer.mPostProcessingOptions.mInvertColours);
                ImGui::Checkbox("Grayscale", &mOpenGLRenderer.mPostProcessingOptions.mGrayScale);
                ImGui::Checkbox("Sharpen", &mOpenGLRenderer.mPostProcessingOptions.mSharpen);
                ImGui::Checkbox("Blur", &mOpenGLRenderer.mPostProcessingOptions.mBlur);
                ImGui::Checkbox("Edge detection", &mOpenGLRenderer.mPostProcessingOptions.mEdgeDetection);

                const bool isPostProcessingOn = mOpenGLRenderer.mPostProcessingOptions.mInvertColours
                    || mOpenGLRenderer.mPostProcessingOptions.mGrayScale || mOpenGLRenderer.mPostProcessingOptions.mSharpen
                    || mOpenGLRenderer.mPostProcessingOptions.mBlur      || mOpenGLRenderer.mPostProcessingOptions.mEdgeDetection;

                if (!isPostProcessingOn) ImGui::BeginDisabled();
                    ImGui::SliderFloat("Kernel offset", &mOpenGLRenderer.mPostProcessingOptions.mKernelOffset, -1.f, 1.f);
                if (!isPostProcessingOn) ImGui::EndDisabled();

                ImGui::TreePop();
            }
            ImGui::Separator();

            ImGui::Checkbox("Force clear colour", &mOpenGLRenderer.mDebugOptions.mForceClearColour);
            if (!mOpenGLRenderer.mDebugOptions.mForceClearColour) ImGui::BeginDisabled();
            {
                ImGui::ColorEdit4("Window clear colour", glm::value_ptr(mOpenGLRenderer.mDebugOptions.mClearColour) );
            }
            if (!mOpenGLRenderer.mDebugOptions.mForceClearColour) ImGui::EndDisabled();

            ImGui::Checkbox("Show light positions", &mOpenGLRenderer.mDebugOptions.mShowLightPositions);
            ImGui::Checkbox("Visualise normals", &mOpenGLRenderer.mDebugOptions.mVisualiseNormals);

            { // Depth options
                ImGui::Checkbox("Force depth test type", &mOpenGLRenderer.mDebugOptions.mForceDepthTestType);

                if (!mOpenGLRenderer.mDebugOptions.mForceDepthTestType) ImGui::BeginDisabled();
                {
                    const std::vector<std::pair<GLType::DepthTestType, const char*>> depthTestOptions =
                    {
                        { GLType::DepthTestType::Always, "Always" },
                        { GLType::DepthTestType::Never, "Never" },
                        { GLType::DepthTestType::Less, "Less" },
                        { GLType::DepthTestType::Equal, "Equal" },
                        { GLType::DepthTestType::NotEqual, "NotEqual" },
                        { GLType::DepthTestType::Greater, "Greater" },
                        { GLType::DepthTestType::LessEqual, "LessEqual" },
                        { GLType::DepthTestType::GreaterEqual, "GreaterEqual" }
                    };
                    ImGui::ComboContainer("Forced depth test type", mOpenGLRenderer.mDebugOptions.mForcedDepthTestType, depthTestOptions );
                }
                if (!mOpenGLRenderer.mDebugOptions.mForceDepthTestType) ImGui::EndDisabled();
            }
            { // Blending options
                ImGui::Checkbox("Force blend type", &mOpenGLRenderer.mDebugOptions.mForceBlendType);

                const std::vector<std::pair<GLType::BlendFactorType, const char*>> blendOptions =
                {
                    { GLType::BlendFactorType::Zero, "Zero" },
                    { GLType::BlendFactorType::One, "One" },
                    { GLType::BlendFactorType::SourceColour, "SourceColour" },
                    { GLType::BlendFactorType::OneMinusSourceColour, "OneMinusSourceColour" },
                    { GLType::BlendFactorType::DestinationColour, "DestinationColour" },
                    { GLType::BlendFactorType::OneMinusDestinationColour, "OneMinusDestinationColour" },
                    { GLType::BlendFactorType::SourceAlpha, "SourceAlpha" },
                    { GLType::BlendFactorType::OneMinusSourceAlpha, "OneMinusSourceAlpha" },
                    { GLType::BlendFactorType::DestinationAlpha, "DestinationAlpha" },
                    { GLType::BlendFactorType::OneMinusDestinationAlpha, "OneMinusDestinationAlpha" },
                    { GLType::BlendFactorType::ConstantColour, "ConstantColour" },
                    { GLType::BlendFactorType::OneMinusConstantColour, "OneMinusConstantColour" },
                    { GLType::BlendFactorType::ConstantAlpha, "ConstantAlpha" },
                    { GLType::BlendFactorType::OneMinusConstantAlpha, "OneMinusConstantAlpha" }
                };
                if (!mOpenGLRenderer.mDebugOptions.mForceBlendType) ImGui::BeginDisabled();
                {
                    ImGui::ComboContainer("Source", mOpenGLRenderer.mDebugOptions.mForcedSourceBlendType, blendOptions );
                    ImGui::ComboContainer("Destination", mOpenGLRenderer.mDebugOptions.mForcedDestinationBlendType, blendOptions );
                }
                if (!mOpenGLRenderer.mDebugOptions.mForceBlendType) ImGui::EndDisabled();
            }
            { // Cull face options
                const std::vector<std::pair<GLType::CullFacesType, const char*>> cullFaceOptions =
                {
                    { GLType::CullFacesType::Back, "Back" },
                    { GLType::CullFacesType::Front, "Front" },
                    { GLType::CullFacesType::FrontAndBack, "FrontAndBack" }
                };

                ImGui::Checkbox("Force cull face type", &mOpenGLRenderer.mDebugOptions.mForceCullFacesType);
                if (!mOpenGLRenderer.mDebugOptions.mForceCullFacesType) ImGui::BeginDisabled();
                {
                    ImGui::ComboContainer("Forced cull faces type", mOpenGLRenderer.mDebugOptions.mForcedCullFacesType, cullFaceOptions );
                }
                if (!mOpenGLRenderer.mDebugOptions.mForceCullFacesType) ImGui::EndDisabled();

                const std::vector<std::pair<GLType::FrontFaceOrientation, const char*>> frontFaceOrientationOptions =
                {
                    { GLType::FrontFaceOrientation::Clockwise, "Clockwise" },
                    { GLType::FrontFaceOrientation::CounterClockwise, "CounterClockwise" }
                };

                ImGui::Checkbox("Force front face type", &mOpenGLRenderer.mDebugOptions.mForceFrontFaceOrientationType);
                if (!mOpenGLRenderer.mDebugOptions.mForceFrontFaceOrientationType) ImGui::BeginDisabled();
                {
                    ImGui::ComboContainer("Forced front face type", mOpenGLRenderer.mDebugOptions.mForcedFrontFaceOrientationType, frontFaceOrientationOptions );
                }
                if (!mOpenGLRenderer.mDebugOptions.mForceFrontFaceOrientationType) ImGui::EndDisabled();
            }

            if (ImGui::Button("Reset"))
            {
                mOpenGLRenderer.mDebugOptions.mShowLightPositions = false;
                mOpenGLRenderer.mDebugOptions.mVisualiseNormals = false;
                mOpenGLRenderer.mDebugOptions.mForceClearColour = false;
                mOpenGLRenderer.mDebugOptions.mClearColour = glm::vec4(0.f, 0.f, 0.f, 0.f);
                mOpenGLRenderer.mDebugOptions.mForceDepthTestType = false;
                mOpenGLRenderer.mDebugOptions.mForcedDepthTestType = GLType::DepthTestType::Less;
                mOpenGLRenderer.mDebugOptions.mForceBlendType = false;
                mOpenGLRenderer.mDebugOptions.mForcedSourceBlendType = GLType::BlendFactorType::SourceAlpha;
                mOpenGLRenderer.mDebugOptions.mForcedDestinationBlendType = GLType::BlendFactorType::OneMinusSourceAlpha;
                mOpenGLRenderer.mDebugOptions.mForceCullFacesType = false;
                mOpenGLRenderer.mDebugOptions.mForcedCullFacesType = GLType::CullFacesType::Back;
                mOpenGLRenderer.mDebugOptions.mForceFrontFaceOrientationType = false;
                mOpenGLRenderer.mDebugOptions.mForcedFrontFaceOrientationType = GLType::FrontFaceOrientation::CounterClockwise;
            }

            // TODO: Draw depth buffer in a box #45
            //mBufferDrawType = static_cast<BufferDrawType>(i);
        }
        ImGui::End();
    }

    void Editor::drawPerformanceWindow()
    {
        if (ImGui::Begin("Performance", &mWindowsToDisplay.Performance))
        {
        }
        ImGui::End();
    }

    void Editor::drawPhysicsWindow()
    {
        if (ImGui::Begin("Physics", &mWindowsToDisplay.Physics))
        {
            ImGui::Checkbox("Show collision geometry", &mOpenGLRenderer.mDebugOptions.mShowCollisionGeometry);
            ImGui::Checkbox("Show orientations", &mOpenGLRenderer.mDebugOptions.mShowOrientations);
            ImGui::Checkbox("Show bounding boxes", &mOpenGLRenderer.mDebugOptions.mShowBoundingBoxes);

            if (!mOpenGLRenderer.mDebugOptions.mShowBoundingBoxes) ImGui::BeginDisabled();
            ImGui::Checkbox("Fill bounding boxes ", &mOpenGLRenderer.mDebugOptions.mFillBoundingBoxes);
            if (!mOpenGLRenderer.mDebugOptions.mShowBoundingBoxes) ImGui::EndDisabled();
        }
        ImGui::End();
    }

void Editor::log(const std::string& p_message)
{
    console.add_log({p_message});
}
void Editor::log_warning(const std::string& p_message)
{
    console.add_log({p_message, glm::vec3(1.f, 1.f, 0.f)});
}
void Editor::log_error(const std::string& p_message)
{
    console.add_log({p_message, glm::vec3(1.f, 0.f, 0.f)});
}

} // namespace UI