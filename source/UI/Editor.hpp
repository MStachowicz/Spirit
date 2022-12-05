#pragma once

namespace System
{
    class SceneSystem;
}
namespace OpenGL
{
    class OpenGLRenderer;
}

namespace UI
{
    // The engine editor overlaying the window/scene when in non-release mode.
    // Allows editing all entity components via ImGui windows and popups.
    class Editor
    {
    public:
        Editor(System::SceneSystem& pSceneSystem, OpenGL::OpenGLRenderer& pOpenGLRenderer);

        void draw();

        int mDrawCount;
    private:
        System::SceneSystem& mSceneSystem;
        OpenGL::OpenGLRenderer& mOpenGLRenderer;

        void drawEntityPanel();
        void drawGraphicsPanel();
        void drawPerformancePanel();
    };
} // namespace UI