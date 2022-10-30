cmake_minimum_required(VERSION 3.6)
# Setup the preprocessor debug and release defines based on the configuartion selected in CMakePresets.json
add_compile_options("$<$<CONFIG:DEBUG>:-DZEPHYR_CONFIG_DEBUG>$<$<CONFIG:RELEASE>:-DZEPHYR_CONFIG_RELEASE>")
# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Zephyr ------------------------------------------------------------------------------------------
project(Zephyr)
add_executable(Zephyr
source/Zephyr.cpp
source/Application.hpp
source/Application.cpp
)
target_link_libraries(Zephyr
PUBLIC System
PRIVATE Renderer
PRIVATE Input
PRIVATE Utility
)
# Zephyr end --------------------------------------------------------------------------------------

# ENTITY COMPONENT SYSTEM *************************************************************************
    # Entity --------------------------------------------------------------------------------------
    add_library(Entity
    source/ECS/Entity/Entity.hpp
    source/ECS/Entity/Entity.cpp
    )
    target_include_directories(Entity
    PUBLIC source/ECS/Entity
    )
    # Entity --------------------------------------------------------------------------------------

    # Component -----------------------------------------------------------------------------------
    add_library(Component
    source/ECS/Component/BoundingBox.cpp
    source/ECS/Component/BoundingBox.hpp
    source/ECS/Component/Camera.cpp
    source/ECS/Component/Camera.hpp
    source/ECS/Component/Collider.cpp
    source/ECS/Component/Collider.hpp
    source/ECS/Component/DirectionalLight.cpp
    source/ECS/Component/DirectionalLight.hpp
    source/ECS/Component/Mesh.cpp
    source/ECS/Component/Mesh.hpp
    source/ECS/Component/PointLight.cpp
    source/ECS/Component/PointLight.hpp
    source/ECS/Component/SpotLight.cpp
    source/ECS/Component/SpotLight.hpp
    source/ECS/Component/Texture.hpp
    source/ECS/Component/Transform.cpp
    source/ECS/Component/Transform.hpp
    )
    target_include_directories(Component
    PUBLIC source/ECS/Component
    PUBLIC source/External/GLM
    )
    target_link_libraries(Component
    PRIVATE ImGui
    )
    # Component end -------------------------------------------------------------------------------

    # System --------------------------------------------------------------------------------------
    add_library(System
    source/ECS/System/Collision/BoundingBoxTree.cpp
    source/ECS/System/Collision/BoundingBoxTree.hpp
    source/ECS/System/Collision/CollisionSystem.cpp
    source/ECS/System/Collision/CollisionSystem.hpp
    source/ECS/System/CameraSystem.cpp
    source/ECS/System/CameraSystem.hpp
    source/ECS/System/ComponentManager.hpp
    source/ECS/System/EntitySystem.cpp
    source/ECS/System/EntitySystem.hpp
    source/ECS/System/EventDispatcher.hpp
    source/ECS/System/MeshSystem.cpp
    source/ECS/System/MeshSystem.hpp
    source/ECS/System/TextureSystem.cpp
    source/ECS/System/TextureSystem.hpp
    )
    target_include_directories(System
    PUBLIC source/ECS/System
    PUBLIC source/ECS/System/Collision
    PRIVATE source/External/stb
    PRIVATE source/External/ASSIMP/include
    )
    target_link_libraries(System
    PUBLIC Component
    PUBLIC Entity
    PUBLIC Utility # ComponentManager.hpp includes Logger (templated class)
    PRIVATE ImGui
    PRIVATE assimp
    )
    # System end ----------------------------------------------------------------------------------
# ENTITY COMPONENT SYSTEM END *********************************************************************

# INPUT & RENDERING BACKEND ***********************************************************************
    # Renderer ------------------------------------------------------------------------------------
    add_library(Renderer
    source/Renderer/Renderer.hpp
    source/Renderer/Renderer.cpp
    source/Renderer/ImGuiConfig.hpp
    )
    target_include_directories(Renderer
    PUBLIC source/Renderer
    )
    target_link_libraries(Renderer
    PUBLIC OpenGL
    PRIVATE System
    PRIVATE Utility
    PRIVATE ImGui
    )
    # Renderer end --------------------------------------------------------------------------------

    # OpenGL --------------------------------------------------------------------------------------
    add_library(OpenGL
    source/Renderer/OpenGL/OpenGLRenderer.hpp
    source/Renderer/OpenGL/OpenGLRenderer.cpp
    source/Renderer/OpenGL/GLState.hpp
    source/Renderer/OpenGL/GLState.cpp
    source/Renderer/OpenGL/OpenGLWindow.hpp
    source/Renderer/OpenGL/OpenGLWindow.cpp
    source/Renderer/OpenGL/Shader.hpp
    source/Renderer/OpenGL/Shader.cpp
    )
    target_include_directories(OpenGL
    PUBLIC source/Renderer/OpenGL
    PUBLIC source/External/GLM
    PUBLIC source/ECS
    PUBLIC source/Data
    PRIVATE source/External/GLFW/include
    )
    target_link_libraries(OpenGL
    PUBLIC System
    PUBLIC Utility # ToIndex used in header for size of Shader::attribute arrays.
    PRIVATE glad_GL
    PRIVATE glfw
    PRIVATE ImGui
    )
    # OpenGL end ----------------------------------------------------------------------------------

    # Input ---------------------------------------------------------------------------------------
    add_library(Input
    source/Input/Input.hpp
    source/Input/Input.cpp
    source/Input/InputAPI.hpp
    source/Input/GLFWInput.hpp
    source/Input/GLFWInput.cpp
    )
    target_include_directories(Zephyr
    PUBLIC source/Input
    PRIVATE source/External/GLFW/include
    )
    target_link_libraries(Input
    PRIVATE glfw
    PRIVATE OpenGL # Zephyr input creates an instance of GLFWInput provided by OpenGLRenderer
    PRIVATE System
    PRIVATE ImGui
    PRIVATE Utility
    )
# Input end ---------------------------------------------------------------------------------------

# STATELESS UTILITY *******************************************************************************
    # ------------------------ Utility ------------------------------------------------------------
    add_library(Utility
    source/Utility/JobSystem/JobSystem.cpp
    source/Utility/JobSystem/JobSystem.hpp
    source/Utility/JobSystem/Test.hpp
    source/Utility/File.cpp
    source/Utility/File.hpp
    source/Utility/Logger.hpp
    source/Utility/Logger.cpp
    source/Utility/Stopwatch.hpp
    source/Utility/Utility.cpp
    source/Utility/Utility.hpp
    )
    target_include_directories(Utility
    PUBLIC source/Utility
    PUBLIC source/Utility/JobSystem
    PUBLIC source/External/GLM
    PRIVATE source/External/spdlog/include
    )
    target_link_libraries(Utility
    PUBLIC spdlog # Logger.hpp uses spdlog in the header.
    )
    # Utility end ---------------------------------------------------------------------------------

# EXTERNAL LIBRARIES ******************************************************************************
    # ImGui ---------------------------------------------------------------------------------------
    # ImGui has no cMakeLists.txt file so need to generate a library for it
    add_library(ImGui
    source/External/imgui/imgui_demo.cpp
    source/External/imgui/imgui_draw.cpp
    source/External/imgui/imgui_tables.cpp
    source/External/imgui/imgui_widgets.cpp
    source/External/imgui/imgui.cpp
    source/External/imgui/backends/imgui_impl_opengl3.cpp
    source/External/imgui/backends/imgui_impl_glfw.cpp
    )
    target_include_directories(ImGui
    PUBLIC source/External/imgui
    PRIVATE source/External/GLFW/include
    )
    # ImGui end -----------------------------------------------------------------------------------

    # spdlog --------------------------------------------------------------------------------------
    add_subdirectory(source/External/spdlog External/spdlog)
    # spdlog end ----------------------------------------------------------------------------------

    # GLFW ----------------------------------------------------------------------------------------
    add_subdirectory(source/External/GLFW External/GLFW)
    # GLFW end ------------------------------------------------------------------------------------

    # GLAD ----------------------------------------------------------------------------------------
    add_subdirectory(source/External/GLAD2/cmake External/GLAD2)
    glad_add_library(glad_GL REPRODUCIBLE MX MXGLOBAL API gl:core=4.3)
    # GLAD end ------------------------------------------------------------------------------------

    # GLM -----------------------------------------------------------------------------------------
    add_subdirectory(source/External/GLM)
    # GLM end -------------------------------------------------------------------------------------

    # ASSIMP --------------------------------------------------------------------------------------
    add_subdirectory(source/External/ASSIMP External/ASSIMP)
    # ASSIMP end ----------------------------------------------------------------------------------