cmake_minimum_required(VERSION 3.6)
# Setup the preprocessor debug and release defines based on the configuartion selected in CMakePresets.json
add_compile_options("$<$<CONFIG:DEBUG>:-DZEPHYR_CONFIG_DEBUG>$<$<CONFIG:RELEASE>:-DZEPHYR_CONFIG_RELEASE>")
# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ------------------------ ZEPHYR ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Zephyr executable")
project(Zephyr)

add_executable(Zephyr
source/Zephyr.cpp
source/Application.hpp
source/Application.cpp
)
target_link_libraries(Zephyr
PRIVATE Logger
PRIVATE JobSystem
PRIVATE FileSystem
PRIVATE Renderer
PRIVATE Input
)
# ------------------------ ZEPHYR END ----------------------------

# ------------------------ RENDERER ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Renderer static library")
add_library(Renderer
source/Renderer/Renderer.hpp
source/Renderer/Renderer.cpp
source/Renderer/MeshManager.hpp
source/Renderer/MeshManager.cpp
source/Renderer/TextureManager.hpp
source/Renderer/TextureManager.cpp
source/Renderer/Camera.hpp
source/Renderer/Camera.cpp
source/Renderer/ImGuiConfig.hpp
)
target_include_directories(Renderer
PUBLIC source/Renderer
PRIVATE source/External/stb
PRIVATE source/External/ASSIMP/include
)
target_link_libraries(Renderer
PUBLIC ECS
PUBLIC OpenGLAPI
PRIVATE FileSystem
PRIVATE Logger
PRIVATE ImGui
PRIVATE assimp
)
# ------------------------ RENDERER END ----------------------------

# ------------------------ OPENGLAPI ---------------------------
# This library implements a Zephyr renderer using GLAD OpenGL as well as an Input handler using GLFW
add_library(OpenGLAPI
source/OpenGLAPI/OpenGLAPI.hpp
source/OpenGLAPI/OpenGLAPI.cpp
source/OpenGLAPI/GLState.hpp
source/OpenGLAPI/GLState.cpp
source/OpenGLAPI/OpenGLWindow.hpp
source/OpenGLAPI/OpenGLWindow.cpp
source/OpenGLAPI/GLFWInput.hpp
source/OpenGLAPI/GLFWInput.cpp
source/OpenGLAPI/Shader.hpp
source/OpenGLAPI/Shader.cpp
)
target_include_directories(OpenGLAPI
PUBLIC source/OpenGLAPI
PRIVATE source/External/GLFW/include
)
target_link_libraries(OpenGLAPI
PUBLIC ZephyrInterface # Implements the InputAPI and GraphicsAPI interface
PRIVATE glad_GL
PRIVATE glfw
PRIVATE ImGui
PRIVATE FileSystem
PRIVATE Logger
)
# ------------------------ OPENGLAPI END ----------------------------

# ------------------------ INPUT ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Input static library")
add_library(Input
source/Input/Input.hpp
source/Input/Input.cpp
)
target_include_directories(Zephyr
PUBLIC source/Input
)
target_link_libraries(Input
PRIVATE OpenGLAPI # Zephyr input creates an instance of GLFWInput provided by OpenGLAPI
PRIVATE Renderer # Included to manipulate Renderer::Camera
PRIVATE ImGui
PRIVATE Logger
)
# ------------------------ INPUT END ----------------------------

# ------------------------ FILESYSTEM ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding File System static library")
add_library(FileSystem
source/FileSystem/FileSystem.hpp
source/FileSystem/FileSystem.cpp
)
target_include_directories(FileSystem
PUBLIC source/FileSystem
)
target_link_libraries(FileSystem
PRIVATE Logger
)
# ------------------------ FILESYSTEM END ----------------------------

# ------------------------ JOBSYSTEM ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Job System static library")
add_library(JobSystem
source/JobSystem/JobSystem.hpp
source/JobSystem/JobSystem.cpp
source/JobSystem/Test.hpp
)
target_include_directories(JobSystem
PUBLIC source/JobSystem # Linking JobSystem exposes its include directory
)
target_link_libraries(JobSystem
PRIVATE Logger
)
# ------------------------ JOBSYSTEM END ----------------------------

# ------------------------ INTERFACE ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Zephyr GraphicsAPI + InputAPI Interfaces")
add_library(ZephyrInterface INTERFACE
source/Interface/DrawCall.hpp
source/Interface/GraphicsAPI.hpp
source/Interface/InputAPI.hpp
)
target_include_directories(ZephyrInterface INTERFACE
INTERFACE source/Interface
)
target_link_libraries(ZephyrInterface
INTERFACE Data
)
# ------------------------ INTERFACE END ----------------------------

# ------------------------ DATA ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Data library")
add_library(Data
source/Data/Mesh.hpp
source/Data/Texture.hpp
source/Data/DirectionalLight.hpp
source/Data/DirectionalLight.cpp
source/Data/PointLight.hpp
source/Data/PointLight.cpp
source/Data/SpotLight.hpp
source/Data/SpotLight.cpp
source/Data/Transform.hpp
source/Data/Transform.cpp
)
target_include_directories(Data
PUBLIC source/Data
PUBLIC source/External/GLM
)
target_link_libraries(Data
INTERFACE Utility
PRIVATE ImGui
)
# ------------------------ DATA END ----------------------------

# ------------------------ ECS ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding ECS interface")
add_library(ECS
source/ECS/Entity.hpp
source/ECS/EntityManager.hpp
source/ECS/EntityManager.cpp
source/ECS/ComponentManager.hpp
)
target_include_directories(ECS
INTERFACE source/ECS
INTERFACE source/External/GLM
)
target_link_libraries(ECS
PRIVATE Logger # Can this be made Private by providing CPP files and making ECS a regular library?
PRIVATE ImGui
PUBLIC Data
)
# ------------------------ ECS END ----------------------------

# ------------------------ UTILITY ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Utility interface")
add_library(Utility INTERFACE)
target_include_directories(Utility
INTERFACE source/Utility
)
# ------------------------ UTILITY END ----------------------------

# ------------------------ LOGGER ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Git spdlog module")
add_subdirectory(source/External/spdlog External/spdlog)

message(STATUS "\n[ZEPHYR] -- Adding Logger static library")
add_library(Logger
source/Logger/Logger.hpp
source/Logger/Logger.cpp
)
target_include_directories(Logger
PUBLIC source/Logger
PRIVATE source/External/spdlog/include
)
target_link_libraries(Logger
PUBLIC spdlog
)
# ------------------------ LOGGER END ----------------------------

# ------------------------ IMGUI ----------------------------
# imgui has no cMakeLists.txt file so need to generate a library for it
message(STATUS "\n[ZEPHYR] -- Generating imgui static library")
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
# ------------------------ IMGUI END ----------------------------

# ------------------------ GLFW ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Git GLFW module")
add_subdirectory(source/External/GLFW External/GLFW)
# ------------------------ GLFW END ----------------------------

# ------------------------ GLAD ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding GLAD2 module")
add_subdirectory(source/External/GLAD2/cmake External/GLAD2)
glad_add_library(glad_GL REPRODUCIBLE MX MXGLOBAL API gl:core=4.3)
# ------------------------ GLAD END ----------------------------

# ------------------------ GLM ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding GLM module")
add_subdirectory(source/External/GLM)
# ------------------------ GLM END ----------------------------

# ------------------------ ASSIMP ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Git ASSIMP module")
add_subdirectory(source/External/ASSIMP External/ASSIMP)
# ------------------------ ASSIMP END ----------------------------

message(STATUS "\n[ZEPHYR] -- Make finished:")