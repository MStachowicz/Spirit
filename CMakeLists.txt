cmake_minimum_required(VERSION 3.22)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
PUBLIC Utility # Simulation loop runs in the header and uses Logger
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
source/Component/Input.cpp
source/Component/Input.hpp
source/Component/Label.hpp
source/Component/Mesh.cpp
source/Component/Mesh.hpp
source/Component/PointLight.cpp
source/Component/PointLight.hpp
source/Component/RigidBody.cpp
source/Component/RigidBody.hpp
source/Component/SpotLight.cpp
source/Component/SpotLight.hpp
source/Component/Texture.cpp
source/Component/Texture.hpp
source/Component/Transform.cpp
source/Component/Transform.hpp
)
target_include_directories(Component
PUBLIC source/Component
PUBLIC source
PRIVATE source/External/stb
PRIVATE source/External/ASSIMP/include
)
target_link_libraries(Component
PUBLIC Geometry
PUBLIC OpenGL
PUBLIC GLM
PUBLIC Utility # Used in headers of Mesh and Texture
PRIVATE Platform
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
PUBLIC ECS #SceneSystem.hpp uses ECS storage
PRIVATE Utility
PRIVATE Platform
)
# System end ----------------------------------------------------------------------------------

# Geometry --------------------------------------------------------------------------------------
add_library(Geometry
source/Geometry/AABB.cpp
source/Geometry/AABB.hpp
source/Geometry/Cylinder.hpp
source/Geometry/Geometry.hpp
source/Geometry/Geometry.cpp
source/Geometry/Intersect.cpp
source/Geometry/Intersect.hpp
source/Geometry/Line.cpp
source/Geometry/Line.hpp
source/Geometry/LineSegment.cpp
source/Geometry/LineSegment.hpp
source/Geometry/Plane.cpp
source/Geometry/Plane.hpp
source/Geometry/Quad.cpp
source/Geometry/Quad.hpp
source/Geometry/Ray.hpp
source/Geometry/Sphere.hpp
source/Geometry/Triangle.hpp
source/Geometry/Triangle.cpp
)
target_include_directories(Geometry
PUBLIC source/Geometry
)
target_link_libraries(Geometry
PUBLIC GLM
PRIVATE Utility
)
# Geometry end ----------------------------------------------------------------------------------

# UI ------------------------------------------------------------------------------------
add_library(UI
source/UI/Editor.hpp
source/UI/Editor.cpp
source/UI/Console.hpp
source/UI/Console.cpp
)
target_include_directories(UI
PUBLIC source/UI
PUBLIC source
)
target_link_libraries(UI
PUBLIC Geometry
PRIVATE System
PRIVATE Platform
PRIVATE OpenGL
PRIVATE ECS
PRIVATE Utility
PRIVATE ImGui
)
# UI end --------------------------------------------------------------------------------

# Platform ------------------------------------------------------------------------------------
add_library(Platform
source/Platform/Window.hpp
source/Platform/Window.cpp
source/Platform/Core.hpp
source/Platform/Core.cpp
source/Platform/Input.hpp
source/Platform/Input.cpp
)
target_include_directories(Platform
PUBLIC source/Platform
PRIVATE source/External/GLFW/include
)
target_link_libraries(Platform
PUBLIC GLM
PUBLIC Utility # EventDispatcher in Window and Input hpps.
PRIVATE glfw
PRIVATE glad_GL
PRIVATE ImGui
)
# Platform end --------------------------------------------------------------------------------

# OpenGL --------------------------------------------------------------------------------------
add_library(OpenGL
source/OpenGL/OpenGLRenderer.hpp
source/OpenGL/OpenGLRenderer.cpp
source/OpenGL/GridRenderer.hpp
source/OpenGL/GridRenderer.cpp
source/OpenGL/DebugRenderer.hpp
source/OpenGL/DebugRenderer.cpp
source/OpenGL/GLState.hpp
source/OpenGL/GLState.cpp
source/OpenGL/Types.hpp
source/OpenGL/Types.cpp
source/OpenGL/Shader.hpp
source/OpenGL/Shader.cpp
)
target_include_directories(OpenGL
PUBLIC source/OpenGL
PRIVATE source/External/GLFW/include
)
target_link_libraries(OpenGL
PRIVATE Utility # ToIndex used in header for size of Shader::attribute arrays.
PUBLIC Geometry
PUBLIC GLM
PRIVATE System
PRIVATE Platform
PRIVATE Component
PRIVATE ECS
PRIVATE glad_GL
PRIVATE glfw
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
PRIVATE source/External/stb
)
target_link_libraries(Utility
PUBLIC GLM
PUBLIC Geometry
PRIVATE UI # Logger.cpp uses Editor for output
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
)
target_link_libraries(Test
PRIVATE Utility
PRIVATE ECS
PRIVATE Geometry
PRIVATE GLM
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
    source/External/ImGuiUser/imgui_user.h
    source/External/ImGuiUser/imgui_user_config.h
    )
    target_include_directories(ImGui
    PUBLIC source/External/imgui
    PUBLIC source/External/ImGuiUser # Seperate folder to avoid comitting to ImGui
    PRIVATE source/External/GLFW/include
    )
    target_compile_definitions(ImGui
    PUBLIC IMGUI_USER_CONFIG="imgui_user_config.h")

    target_link_libraries(ImGui
    PUBLIC GLM
    )
    # ImGui end -----------------------------------------------------------------------------------

    # GLFW ----------------------------------------------------------------------------------------
    # Dont build any examples, tests or documentation
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    add_subdirectory(source/External/GLFW External/GLFW)
    # GLFW end ------------------------------------------------------------------------------------

    # GLAD ----------------------------------------------------------------------------------------
    add_subdirectory(source/External/GLAD2/cmake External/GLAD2)
    glad_add_library(glad_GL API gl:core=4.3)
    # GLAD end ------------------------------------------------------------------------------------

    # GLM -----------------------------------------------------------------------------------------
    add_library(GLM INTERFACE)
    target_include_directories(GLM INTERFACE source/External/GLM)
    # GLM end -------------------------------------------------------------------------------------

    # ASSIMP --------------------------------------------------------------------------------------
    # Remove all supported filetypes for import and export.
    # Enable only OBJ files for import.
    set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
    add_subdirectory(source/External/ASSIMP External/ASSIMP)
    # ASSIMP end ----------------------------------------------------------------------------------

    # STB included (header only)