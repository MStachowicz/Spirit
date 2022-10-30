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
PRIVATE Utility
PRIVATE Renderer
PRIVATE Input
)
# ------------------------ ZEPHYR END ----------------------------

# ------------------------ COLLISION ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Collision static library")
add_library(Collision
source/Collision/BoundingBox.hpp
source/Collision/BoundingBox.cpp
source/Collision/BoundingBoxTree.hpp
source/Collision/BoundingBoxTree.cpp
source/Collision/CollisionSystem.hpp
source/Collision/CollisionSystem.cpp
)
target_include_directories(Collision
PUBLIC source/Collision
PRIVATE source/ECS
)
target_link_libraries(Collision
PRIVATE ECS
)
# ------------------------ COLLISION END ----------------------------

# ------------------------ RENDERER ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Renderer static library")
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
PRIVATE ECS
PRIVATE Logger
PRIVATE ImGui
)
# ------------------------ RENDERER END ----------------------------

# ------------------------ OpenGL ---------------------------
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
PUBLIC Utility
PUBLIC ECS
PUBLIC Data
PRIVATE glad_GL
PRIVATE glfw
PRIVATE ImGui
PRIVATE Logger
)
# ------------------------ OPENGL END ----------------------------

# ------------------------ DATA ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Data library")
add_library(Data
source/Data/Mesh.hpp
source/Data/Mesh.cpp
source/Data/Collider.hpp
source/Data/Collider.cpp
source/Data/Camera.hpp
source/Data/Camera.cpp
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
PUBLIC source/Collision
)
target_link_libraries(Data
PUBLIC Utility
PUBLIC Collision
PRIVATE ImGui
)
# ------------------------ DATA END ----------------------------

# ------------------------ ECS ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding ECS interface")
add_library(ECS
source/ECS/Entity.hpp
source/ECS/EntityManager.hpp
source/ECS/EntityManager.cpp
source/ECS/Managers/MeshManager.hpp
source/ECS/Managers/MeshManager.cpp
source/ECS/Managers/CameraManager.hpp
source/ECS/Managers/CameraManager.cpp
source/ECS/Managers/TextureManager.hpp
source/ECS/Managers/TextureManager.cpp
source/ECS/ComponentManager.hpp
)
target_include_directories(ECS
PUBLIC source/ECS
PUBLIC source/External/GLM
PRIVATE source/External/stb
PRIVATE source/External/ASSIMP/include
)
target_link_libraries(ECS
PUBLIC Data
PRIVATE Logger
PRIVATE Utility
PRIVATE assimp
PRIVATE ImGui
)
# ------------------------ ECS END ----------------------------

# ------------------------ INPUT ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Input static library")
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
PRIVATE ECS
PRIVATE ImGui
PRIVATE Logger
)
# ------------------------ INPUT END ----------------------------

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

# ------------------------ UTILITY ----------------------------
message(STATUS "\n[ZEPHYR] -- Adding Utility static library")
add_library(Utility
source/Utility/Utility.hpp
source/Utility/Utility.cpp
source/Utility/File.hpp
source/Utility/File.cpp
source/Utility/Types.hpp
source/Utility/Stopwatch.hpp
source/Utility/EventDispatcher.hpp
)
target_include_directories(Utility
PUBLIC source/Utility
PUBLIC source/External/GLM
)
target_link_libraries(Utility
PUBLIC Logger
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