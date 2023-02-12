cmake_minimum_required(VERSION 3.6)
# Setup the preprocessor debug and release defines based on the configuartion selected in CMakePresets.json
add_compile_options("$<$<CONFIG:DEBUG>:-DZEPHYR_CONFIG_DEBUG>$<$<CONFIG:RELEASE>:-DZEPHYR_CONFIG_RELEASE>")
# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Zephyr ------------------------------------------------------------------------------------------
project(Zephyr)
add_executable(Zephyr
source/Application.hpp
source/Application.cpp
)
target_link_libraries(Zephyr
PUBLIC System
PUBLIC OpenGL
PUBLIC Platform
PUBLIC UI
PUBLIC Utility
PUBLIC Test
)
# Zephyr end --------------------------------------------------------------------------------------

# ECS --------------------------------------------------------------------------------------
add_library(ECS
source/ECS/Storage.hpp
source/ECS/Storage.cpp
source/ECS/Meta.hpp
)
target_include_directories(ECS
PUBLIC source/ECS
)
target_link_libraries(ECS
PUBLIC Utility # Uses Logger in Storage.hpp
)
# ECS --------------------------------------------------------------------------------------

# Component -----------------------------------------------------------------------------------
add_library(Component
source/Component/Camera.cpp
source/Component/Camera.hpp
source/Component/Collider.cpp
source/Component/Collider.hpp
source/Component/DirectionalLight.cpp
source/Component/DirectionalLight.hpp
source/Component/Mesh.cpp
source/Component/Mesh.hpp
source/Component/Label.hpp
source/Component/PointLight.cpp
source/Component/PointLight.hpp
source/Component/RigidBody.cpp
source/Component/RigidBody.hpp
source/Component/SpotLight.cpp
source/Component/SpotLight.hpp
source/Component/Texture.hpp
source/Component/Texture.cpp
source/Component/Transform.cpp
source/Component/Transform.hpp
)
target_include_directories(Component
PUBLIC source/Component
PUBLIC source/External/GLM
PRIVATE source/External/stb
PRIVATE source/External/ASSIMP/include
)
target_link_libraries(Component
PUBLIC Geometry
PUBLIC OpenGL
PUBLIC Utility
PRIVATE ImGui
PRIVATE assimp
)
# Component end -------------------------------------------------------------------------------

# System --------------------------------------------------------------------------------------
add_library(System
source/System/CollisionSystem.cpp
source/System/CollisionSystem.hpp
source/System/PhysicsSystem.cpp
source/System/PhysicsSystem.hpp
source/System/InputSystem.hpp
source/System/InputSystem.cpp
source/System/SceneSystem.hpp
source/System/SceneSystem.cpp
source/System/MeshSystem.cpp
source/System/MeshSystem.hpp
source/System/TextureSystem.cpp
source/System/TextureSystem.hpp
)
target_include_directories(System
PUBLIC source/System
)
target_link_libraries(System
PUBLIC Component
PUBLIC Geometry
PUBLIC Utility
PUBLIC ECS #SceneSystem.hpp uses ECS storage
PRIVATE Platform
)
# System end ----------------------------------------------------------------------------------

# Geometry --------------------------------------------------------------------------------------
add_library(Geometry
source/Geometry/AABB.cpp
source/Geometry/AABB.hpp
source/Geometry/Cylinder.hpp
source/Geometry/Intersect.cpp
source/Geometry/Intersect.hpp
source/Geometry/Plane.cpp
source/Geometry/Plane.hpp
source/Geometry/Ray.hpp
source/Geometry/Sphere.hpp
source/Geometry/Triangle.hpp
source/Geometry/Triangle.cpp
source/Geometry/Geometry.hpp
source/Geometry/Geometry.cpp
)
target_include_directories(Geometry
PUBLIC source/External/GLM
PUBLIC source/Geometry
)
target_link_libraries(Geometry
)
# Geometry end ----------------------------------------------------------------------------------

# UI ------------------------------------------------------------------------------------
add_library(UI
source/UI/Editor.hpp
source/UI/Editor.cpp
source/UI/ImGuiConfig.hpp
)
target_include_directories(UI
PUBLIC source/UI
)
target_link_libraries(UI
PRIVATE System
PRIVATE Platform
PRIVATE Geometry
PRIVATE OpenGL
PRIVATE ECS
PRIVATE Utility
PRIVATE ImGui
)
# UI end --------------------------------------------------------------------------------

# Platform ------------------------------------------------------------------------------------
add_library(Platform
source/Platform/Core.hpp
source/Platform/Core.cpp
source/Platform/InputDefinitions.hpp
)
target_include_directories(Platform
PUBLIC source/Platform
PRIVATE source/External/GLFW/include
)
target_link_libraries(Platform
PUBLIC Utility
PRIVATE glfw
PRIVATE glad_GL
PRIVATE ImGui
)
# Platform end --------------------------------------------------------------------------------

# OpenGL --------------------------------------------------------------------------------------
add_library(OpenGL
source/OpenGL/OpenGLRenderer.hpp
source/OpenGL/OpenGLRenderer.cpp
source/OpenGL/GLState.hpp
source/OpenGL/GLState.cpp
source/OpenGL/Types.hpp
source/OpenGL/Types.cpp
source/OpenGL/Shader.hpp
source/OpenGL/Shader.cpp
)
target_include_directories(OpenGL
PUBLIC source/OpenGL
PUBLIC source/External/GLM
PRIVATE source/External/GLFW/include
)
target_link_libraries(OpenGL
PUBLIC Utility # ToIndex used in header for size of Shader::attribute arrays.
PUBLIC Geometry
PRIVATE System
PRIVATE Platform
PRIVATE Component
PRIVATE ECS
PRIVATE glad_GL
PRIVATE glfw
PRIVATE ImGui
)
# OpenGL end ----------------------------------------------------------------------------------

# ------------------------ Utility ------------------------------------------------------------
add_library(Utility
source/Utility/EventDispatcher.hpp
source/Utility/JobSystem/JobSystem.cpp
source/Utility/JobSystem/JobSystem.hpp
source/Utility/JobSystem/Test.hpp
source/Utility/ResourceManager.hpp
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

# ------------------------ Test ------------------------------------------------------------
add_library(Test
source/Test/MemoryCorrectnessItem.hpp
source/Test/MemoryCorrectnessItem.cpp
source/Test/TestManager.hpp
source/Test/TestManager.cpp
source/Test/Tests/ECSTester.hpp
source/Test/Tests/ECSTester.cpp
source/Test/Tests/GeometryTester.hpp
source/Test/Tests/GeometryTester.cpp
)
target_include_directories(Test
PUBLIC source/Test
PUBLIC source/Test/Tests
PUBLIC source/External/GLM
)
target_link_libraries(Test
PRIVATE Utility
PRIVATE ECS
PRIVATE Geometry
)
# Test end ---------------------------------------------------------------------------------

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

    # STB included (header only)