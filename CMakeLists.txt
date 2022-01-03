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
target_include_directories(Zephyr
PRIVATE source/Renderer
PRIVATE source/FileSystem
PRIVATE source/Input
)
target_link_libraries(Zephyr
Logger
JobSystem
FileSystem
Renderer
Input
)
# ------------------------ ZEPHYR ----------------------------

# ------------------------ RENDERER ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Renderer static library")
add_library(Renderer
source/Renderer/Renderer.hpp
source/Renderer/Renderer.cpp
source/Renderer/Camera.hpp
source/Renderer/Camera.cpp
source/Renderer/GraphicsAPI.hpp
source/Renderer/ImGuiConfig.hpp
)
target_include_directories(Renderer
PRIVATE source/ECS
PRIVATE source/External/GLM
PRIVATE source/External/imgui
)
target_link_libraries(Renderer
ImGui
OpenGLAPI
Logger
ECS
)
# ------------------------ RENDERER ----------------------------

# ------------------------ INPUT ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Input static library")
add_library(Input
source/Input/Input.hpp
source/Input/Input.cpp
source/Input/InputAPI.hpp
)
target_include_directories(Input
PRIVATE source/Renderer
)
target_link_libraries(Input
OpenGLAPI # OpenGLAPI also includes a GLFWInput satisfying the InputAPI interface
Logger
)
# ------------------------ INPUT ----------------------------

# ------------------------ Job System ----------------------------
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
Logger
)
# ------------------------ Job System ----------------------------

# ------------------------ File System ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding File System static library")
add_library(FileSystem
source/FileSystem/FileSystem.hpp
source/FileSystem/FileSystem.cpp
)
target_include_directories(FileSystem
PRIVATE source/FileSystem
PRIVATE source/External/stb
)
target_link_libraries(FileSystem
Logger
)
# ------------------------ File System ----------------------------

# ------------------------ Entity Component System ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding ECS interface")
add_library(ECS INTERFACE
source/ECS/Entity.hpp
source/ECS/ComponentManager.hpp
)
target_include_directories(ECS
INTERFACE source/ECS
)
target_link_libraries(ECS
INTERFACE Logger
)
# ------------------------ Entity Component System ----------------------------

# ------------------------ LOGGER ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Git spdlog module")
add_subdirectory(source/External/spdlog External/spdlog)

message(STATUS "\n[ZEPHYR] -- Adding Logger static library")
add_library(Logger
source/Logger/Logger.hpp
source/Logger/Logger.cpp
)
target_include_directories(Logger
PRIVATE source/External/spdlog/include
PUBLIC source/Logger # Linking Logger exposes its include directory
)
target_link_libraries(Logger
spdlog
)
# ------------------------ LOGGER ----------------------------



# ------------------------ OPENGL API ---------------------------
# This library implements a Zephyr renderer using GLAD OpenGL as well as an Input handler using GLFW
add_library(OpenGLAPI
source/Renderer/GraphicsAPI.hpp # implements the Zephyr::Renderer GraphicsAPI interface
source/OpenGLAPI/OpenGLAPI.hpp
source/OpenGLAPI/OpenGLAPI.cpp
source/OpenGLAPI/OpenGLWindow.hpp
source/OpenGLAPI/OpenGLWindow.cpp
source/Input/InputAPI.hpp # implements the Zephyr::Input InputAPI interface
source/OpenGLAPI/GLFWInput.hpp
source/OpenGLAPI/GLFWInput.cpp
)
target_include_directories(OpenGLAPI
PUBLIC source/External/GLM
PUBLIC source/External/imgui
PUBLIC source/OpenGLAPI
PRIVATE source/External/GLFW/include
PRIVATE source/Renderer
PRIVATE source/Input
PRIVATE source/FileSystem
)
target_link_libraries(OpenGLAPI
glad_GL
glfw
ImGui
FileSystem
Logger
)
# ------------------------ OPENGL API END ----------------------------

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
PRIVATE source/External/imgui
PRIVATE source/External/GLFW/include
)
# ------------------------ IMGUI ----------------------------

# ------------------------ GLFW ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Git GLFW module")
add_subdirectory(source/External/GLFW External/GLFW)
# ------------------------ GLFW ----------------------------

# ------------------------ GLAD ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding GLAD2 module")
add_subdirectory(source/External/GLAD2/cmake External/GLAD2)
glad_add_library(glad_GL REPRODUCIBLE MX MXGLOBAL API gl:core=3.3)
# ------------------------ GLAD ----------------------------

# ------------------------ GLM ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding GLM module")
add_subdirectory(source/External/GLM)
# ------------------------ GLM ----------------------------

message(STATUS "\n[ZEPHYR] -- Make finished:")