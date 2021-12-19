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
Renderer
FileSystem
Input
)
# ------------------------ ZEPHYR ----------------------------

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

# ------------------------ RENDERER ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Renderer static library")
add_library(Renderer
source/renderer/Renderer.hpp
source/renderer/Renderer.cpp
)
target_include_directories(Renderer
PRIVATE source/Renderer/GraphicsContext
PRIVATE source/External/GLM
PRIVATE source/ECS
)
target_link_libraries(Renderer
Logger
GraphicsContext
ECS
)
# ------------------------ RENDERER ----------------------------

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

# ------------------------ GRAPHICS CONTEXT ----------------------------
add_library(GraphicsContext
source/Renderer/GraphicsContext/Context.hpp
source/Renderer/GraphicsContext/OpenGLContext.hpp
source/Renderer/GraphicsContext/OpenGLContext.cpp
)
target_include_directories(GraphicsContext
PRIVATE source/External/GLFW/include
PRIVATE source/External/imgui
INTERFACE source/External/GLM   #GLM is an interface as it's included in Context.hpp header
PRIVATE source/Input
PRIVATE source/FileSystem
)
target_link_libraries(GraphicsContext
glad_GL
glfw
ImGui
Input
FileSystem
Logger
)
# ------------------------ GRAPHICS CONTEXT ----------------------------

# ------------------------ INPUT ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Input static library")
add_library(Input
source/Input/Input.hpp
source/Input/Input.cpp
)
target_include_directories(Input
PRIVATE source/Renderer/GraphicsContext
)
target_link_libraries(Input
Logger
GraphicsContext
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