#include "Application.hpp"

Application::Application(Platform::Input& p_input, Platform::Window& p_window) noexcept
    : m_input{p_input}
    , m_window{p_window}
    , mTextureSystem{}
    , mMeshSystem{mTextureSystem}
    , mSceneSystem{mTextureSystem, mMeshSystem}
    , mOpenGLRenderer{m_window, mSceneSystem, mMeshSystem, mTextureSystem}
    , m_grid_renderer{}
    , mCollisionSystem{mSceneSystem, mMeshSystem}
    , mPhysicsSystem{mSceneSystem, mCollisionSystem}
    , mInputSystem{m_input, m_window, mSceneSystem}
    , mEditor{m_input, m_window, mTextureSystem, mMeshSystem, mSceneSystem, mCollisionSystem, mOpenGLRenderer}
    , mSimulationLoopParamsChanged{false}
    , mPhysicsTicksPerSecond{60}
    , mRenderTicksPerSecond{120}
    , m_input_ticks_per_second{120}
    , maxFrameDelta{std::chrono::milliseconds(250)}
{
    Logger::s_editor_sink = &mEditor;
}
Application::~Application() noexcept
{
    Logger::s_editor_sink = nullptr;
}

void Application::simulationLoop()
{
    while (!m_window.close_requested())
    {
        ASSERT(mPhysicsTicksPerSecond == 30   || mPhysicsTicksPerSecond == 60   || mPhysicsTicksPerSecond == 90   || mPhysicsTicksPerSecond == 120,   "[APPLICATION] Physics ticks per second requested invalid!");
        ASSERT(mRenderTicksPerSecond == 30    || mRenderTicksPerSecond == 60    || mRenderTicksPerSecond == 90    || mRenderTicksPerSecond == 120,    "[APPLICATION] Render ticks per second requested invalid!");
        ASSERT(m_input_ticks_per_second == 30 || m_input_ticks_per_second == 60 || m_input_ticks_per_second == 90 || m_input_ticks_per_second == 120, "[APPLICATION] Input ticks per second requested invalid!");

        switch (mPhysicsTicksPerSecond)
        {
            case 30:
            {
                switch (mRenderTicksPerSecond)
                {
                    case 30:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<30, 30, 30>();    break;
                            case 60:  simulationLoop<30, 30, 60>();    break;
                            case 90:  simulationLoop<30, 30, 90>();    break;
                            case 120: simulationLoop<30, 30, 120>();   break;
                        } break;
                    }
                    case 60:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<30, 60, 30>();    break;
                            case 60:  simulationLoop<30, 60, 60>();    break;
                            case 90:  simulationLoop<30, 60, 90>();    break;
                            case 120: simulationLoop<30, 60, 120>();   break;
                        } break;
                    }
                    case 90:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<30, 90, 30>();    break;
                            case 60:  simulationLoop<30, 90, 60>();    break;
                            case 90:  simulationLoop<30, 90, 90>();    break;
                            case 120: simulationLoop<30, 90, 120>();   break;
                        } break;
                    }
                    case 120:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<30, 120, 30>();   break;
                            case 60:  simulationLoop<30, 120, 60>();   break;
                            case 90:  simulationLoop<30, 120, 90>();   break;
                            case 120: simulationLoop<30, 120, 120>();  break;
                        } break;
                    }
                } break;
            }
            case 60:
            {
                switch (mRenderTicksPerSecond)
                {
                    case 30:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<60, 30, 30>();    break;
                            case 60:  simulationLoop<60, 30, 60>();    break;
                            case 90:  simulationLoop<60, 30, 90>();    break;
                            case 120: simulationLoop<60, 30, 120>();   break;
                        } break;
                    }
                    case 60:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<60, 60, 30>();    break;
                            case 60:  simulationLoop<60, 60, 60>();    break;
                            case 90:  simulationLoop<60, 60, 90>();    break;
                            case 120: simulationLoop<60, 60, 120>();   break;
                        } break;
                    }
                    case 90:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<60, 90, 30>();    break;
                            case 60:  simulationLoop<60, 90, 60>();    break;
                            case 90:  simulationLoop<60, 90, 90>();    break;
                            case 120: simulationLoop<60, 90, 120>();   break;
                        } break;
                    }
                    case 120:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<60, 120, 30>();   break;
                            case 60:  simulationLoop<60, 120, 60>();   break;
                            case 90:  simulationLoop<60, 120, 90>();   break;
                            case 120: simulationLoop<60, 120, 120>();  break;
                        } break;
                    }
                } break;
            }
            case 90:
            {
                switch (mRenderTicksPerSecond)
                {
                    case 30:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<90, 30, 30>();    break;
                            case 60:  simulationLoop<90, 30, 60>();    break;
                            case 90:  simulationLoop<90, 30, 90>();    break;
                            case 120: simulationLoop<90, 30, 120>();   break;
                        } break;
                    }
                    case 60:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<90, 60, 30>();    break;
                            case 60:  simulationLoop<90, 60, 60>();    break;
                            case 90:  simulationLoop<90, 60, 90>();    break;
                            case 120: simulationLoop<90, 60, 120>();   break;
                        } break;
                    }
                    case 90:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<90, 90, 30>();    break;
                            case 60:  simulationLoop<90, 90, 60>();    break;
                            case 90:  simulationLoop<90, 90, 90>();    break;
                            case 120: simulationLoop<90, 90, 120>();   break;
                        } break;
                    }
                    case 120:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<90, 120, 30>();   break;
                            case 60:  simulationLoop<90, 120, 60>();   break;
                            case 90:  simulationLoop<90, 120, 90>();   break;
                            case 120: simulationLoop<90, 120, 120>();  break;
                        } break;
                    }
                } break;
            }
            case 120:
            {
                switch (mRenderTicksPerSecond)
                {
                    case 30:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<120, 30, 30>();   break;
                            case 60:  simulationLoop<120, 30, 60>();   break;
                            case 90:  simulationLoop<120, 30, 90>();   break;
                            case 120: simulationLoop<120, 30, 120>();  break;
                        } break;
                    }
                    case 60:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<120, 60, 30>();   break;
                            case 60:  simulationLoop<120, 60, 60>();   break;
                            case 90:  simulationLoop<120, 60, 90>();   break;
                            case 120: simulationLoop<120, 60, 120>();  break;
                        } break;
                    }
                    case 90:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<120, 90, 30>();   break;
                            case 60:  simulationLoop<120, 90, 60>();   break;
                            case 90:  simulationLoop<120, 90, 90>();   break;
                            case 120: simulationLoop<120, 90, 120>();  break;
                        } break;
                    }
                    case 120:
                    {
                        switch (m_input_ticks_per_second)
                        {
                            case 30:  simulationLoop<120, 120, 30>();  break;
                            case 60:  simulationLoop<120, 120, 60>();  break;
                            case 90:  simulationLoop<120, 120, 90>();  break;
                            case 120: simulationLoop<120, 120, 120>(); break;
                        } break;
                    }
                } break;
            }
        }

        // After exiting a simulation loop we may have requested a physics timestep change.
        // Reset this flag to not exit the next simulationLoop when looping back around this While().
        mSimulationLoopParamsChanged = false;
    }
}