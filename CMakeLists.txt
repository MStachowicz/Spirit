cmake_minimum_required(VERSION 3.27)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Zephyr ----------------------------------------------------------------------------------------------------------------------------------
project(Zephyr)
add_executable(Zephyr
source/Application.hpp
source/Application.cpp
)
target_include_directories(Zephyr
PRIVATE source
)
target_link_libraries(Zephyr
PUBLIC System
PUBLIC OpenGL
PUBLIC Platform
PUBLIC UI
PUBLIC Utility # Simulation loop runs in the header and uses Logger
PUBLIC Test
)
target_compile_options(Zephyr PRIVATE ${WARNING_COMPILE_FLAGS})
# Zephyr end ------------------------------------------------------------------------------------------------------------------------------

# Set variables after project() so we can use CMAKE_CXX_COMPILER_ID -----------------------------------------------------------------------
set(SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}) # TODO remove this and use CMAKE_CURRENT_SOURCE_DIR directly
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(OPENGL_VERSION_MAJOR  4)
set(OPENGL_VERSION_MINOR  3)
set(OPENGL_VERSION_STRING "${OPENGL_VERSION_MAJOR}.${OPENGL_VERSION_MINOR}")
set(GLSL_VERSION_STRING   "#version ${OPENGL_VERSION_MAJOR}${OPENGL_VERSION_MINOR}0")

IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
	set(IS_MSVC TRUE)
	set(WARNING_COMPILE_FLAGS /W4; /WX; /external:W0;)
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
	set(IS_GCC TRUE)
	set(WARNING_COMPILE_FLAGS -Wall; -Wextra; -Wpedantic; -Wformat; -Wformat-security; -Werror;)
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(IS_CLANG TRUE)
	set(WARNING_COMPILE_FLAGS -Wall; -Wextra; -Wpedantic; -Wformat; -Wformat-security; -Werror;)
ELSE() # If we could not determine the compile ID match put warning message up
	message(WARNING "Compiler: \"${CMAKE_CXX_COMPILER_ID}\" has not been tested with this project, compilation may fail")
ENDIF()
message (STATUS "Compiling with \"${CMAKE_CXX_COMPILER_ID}\" - Adding compiler flags to non-externals libs: \"${WARNING_COMPILE_FLAGS}\"")

# IF generator is Visual Studio set the startup project to Zephyr -------------------------------------------------------------------------
if (CMAKE_GENERATOR MATCHES "Visual Studio")
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Zephyr) # Makes Zephyr the startup project in VS .sln
endif ()
#------------------------------------------------------------------------------------------------------------------------------------------

# ECS -------------------------------------------------------------------------------------------------------------------------------------
add_library(ECS
source/ECS/Storage.hpp
source/ECS/Storage.cpp
source/ECS/Meta.hpp
)
target_include_directories(ECS
PRIVATE source/ECS
PRIVATE source
)
target_link_libraries(ECS
PUBLIC Utility # Uses Logger in Storage.hpp
)
target_compile_options(ECS PRIVATE ${WARNING_COMPILE_FLAGS})
# ECS -------------------------------------------------------------------------------------------------------------------------------------

# Component -------------------------------------------------------------------------------------------------------------------------------
add_library(Component
source/Component/Camera.cpp
source/Component/Camera.hpp
source/Component/Collider.cpp
source/Component/Collider.hpp
source/Component/Input.cpp
source/Component/Input.hpp
source/Component/Label.hpp
source/Component/Mesh.cpp
source/Component/Mesh.hpp
source/Component/ParticleEmitter.hpp
source/Component/ParticleEmitter.cpp
source/Component/RigidBody.cpp
source/Component/RigidBody.hpp
source/Component/Lights.cpp
source/Component/Lights.hpp
source/Component/Terrain.cpp
source/Component/Terrain.hpp
source/Component/Texture.cpp
source/Component/Texture.hpp
source/Component/Transform.cpp
source/Component/Transform.hpp
source/Component/Vertex.hpp
)
target_include_directories(Component
PRIVATE source/Component
PRIVATE source
)
target_link_libraries(Component
PUBLIC Geometry
PUBLIC OpenGL
PUBLIC GLM
PUBLIC Utility # Used in headers of Mesh, Texture, Input
PRIVATE System
PRIVATE Platform
PRIVATE ImGui
PRIVATE STB
)
target_compile_options(Component PRIVATE ${WARNING_COMPILE_FLAGS})
# Component end ---------------------------------------------------------------------------------------------------------------------------

# System ----------------------------------------------------------------------------------------------------------------------------------
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
PRIVATE source/System
PRIVATE source
)
target_link_libraries(System
PUBLIC Component
PUBLIC Geometry
PUBLIC ECS #SceneSystem.hpp uses ECS storage
PUBLIC Utility
PRIVATE Platform
)
target_compile_options(System PRIVATE ${WARNING_COMPILE_FLAGS})
# System end ------------------------------------------------------------------------------------------------------------------------------

# Geometry --------------------------------------------------------------------------------------------------------------------------------
add_library(Geometry
source/Geometry/AABB.cpp
source/Geometry/AABB.hpp
source/Geometry/Cylinder.hpp
source/Geometry/Cylinder.cpp
source/Geometry/Cone.hpp
source/Geometry/Cone.cpp
source/Geometry/Cuboid.hpp
source/Geometry/Cuboid.cpp
source/Geometry/Geometry.hpp
source/Geometry/Geometry.cpp
source/Geometry/Frustrum.hpp
source/Geometry/Frustrum.cpp
source/Geometry/Intersect.cpp
source/Geometry/Intersect.hpp
source/Geometry/Line.cpp
source/Geometry/Line.hpp
source/Geometry/LineSegment.cpp
source/Geometry/LineSegment.hpp
source/Geometry/Plane.cpp
source/Geometry/Plane.hpp
source/Geometry/Point.hpp
source/Geometry/Quad.cpp
source/Geometry/Quad.hpp
source/Geometry/Ray.hpp
source/Geometry/Sphere.hpp
source/Geometry/Sphere.cpp
source/Geometry/Shape.hpp
source/Geometry/Triangle.hpp
source/Geometry/Triangle.cpp
source/Geometry/TriTri.hpp
source/Geometry/TriTri.cpp
)
target_include_directories(Geometry
PRIVATE source/Geometry
PRIVATE source
)
target_link_libraries(Geometry
PUBLIC GLM
PRIVATE Utility
PRIVATE ImGui
)
target_compile_options(Geometry PRIVATE ${WARNING_COMPILE_FLAGS})
# Geometry end ----------------------------------------------------------------------------------------------------------------------------

# UI --------------------------------------------------------------------------------------------------------------------------------------
add_library(UI
source/UI/Editor.hpp
source/UI/Editor.cpp
source/UI/Console.hpp
source/UI/Console.cpp
)
target_include_directories(UI
PRIVATE source/UI
PRIVATE source
)
target_link_libraries(UI
PUBLIC GLM
PUBLIC ECS
PRIVATE Geometry
PRIVATE System
PRIVATE Platform
PRIVATE OpenGL
PRIVATE Utility
PRIVATE ImGui
PRIVATE Test #Included to easily add calls to visualisation functions in testers
)
target_compile_options(UI PRIVATE ${WARNING_COMPILE_FLAGS})
# UI end ----------------------------------------------------------------------------------------------------------------------------------

# Platform --------------------------------------------------------------------------------------------------------------------------------
add_library(Platform
source/Platform/Window.hpp
source/Platform/Window.cpp
source/Platform/Core.hpp
source/Platform/Core.cpp
source/Platform/Input.hpp
source/Platform/Input.cpp
)
target_include_directories(Platform
PRIVATE source/Platform
PRIVATE source
)
target_link_libraries(Platform
PUBLIC GLM
PUBLIC Utility # EventDispatcher in Window and Input hpps.
PRIVATE glfw
PRIVATE glad
PRIVATE ImGui
)
target_compile_options(Platform PRIVATE ${WARNING_COMPILE_FLAGS})
# Platform end ----------------------------------------------------------------------------------------------------------------------------

# OpenGL ----------------------------------------------------------------------------------------------------------------------------------
add_library(OpenGL
source/OpenGL/DrawCall.hpp
source/OpenGL/DrawCall.cpp
source/OpenGL/OpenGLRenderer.hpp
source/OpenGL/OpenGLRenderer.cpp
source/OpenGL/GridRenderer.hpp
source/OpenGL/GridRenderer.cpp
source/OpenGL/DebugRenderer.hpp
source/OpenGL/DebugRenderer.cpp
source/OpenGL/ParticleRenderer.cpp
source/OpenGL/ParticleRenderer.hpp
source/OpenGL/ShadowMapper.hpp
source/OpenGL/ShadowMapper.cpp
source/OpenGL/GLState.hpp
source/OpenGL/GLState.cpp
source/OpenGL/PhongRenderer.hpp
source/OpenGL/PhongRenderer.cpp
source/OpenGL/Types.hpp
source/OpenGL/Types.cpp
source/OpenGL/Shader.hpp
source/OpenGL/Shader.cpp
)
target_include_directories(OpenGL
PRIVATE source/OpenGL
PRIVATE source
)
target_link_libraries(OpenGL
PUBLIC Utility # Uses MeshBuilder, Logger in headers
PUBLIC GLM
PRIVATE Geometry
PRIVATE System
PRIVATE Platform
PRIVATE Component
PRIVATE ECS
PRIVATE glad
)
target_compile_options(OpenGL PRIVATE ${WARNING_COMPILE_FLAGS})
# OpenGL end ------------------------------------------------------------------------------------------------------------------------------

# Utility ---------------------------------------------------------------------------------------------------------------------------------
configure_file(source/Utility/Config.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/source/Utility/Config.hpp)

add_library(Utility
source/Utility/EventDispatcher.hpp
source/Utility/ResourceManager.hpp
source/Utility/FunctionTraits.hpp
source/Utility/File.cpp
source/Utility/File.hpp
source/Utility/Logger.hpp
source/Utility/Logger.cpp
source/Utility/MeshBuilder.hpp
source/Utility/PerlinNoise.hpp
source/Utility/Stopwatch.hpp
source/Utility/Utility.cpp
source/Utility/Utility.hpp
)
target_include_directories(Utility
PRIVATE source/Utility
PRIVATE source
)
target_link_libraries(Utility
PUBLIC GLM
PUBLIC Geometry
PUBLIC OpenGL
PRIVATE STB
PRIVATE UI # Logger.cpp uses Editor for output
)
target_compile_options(Utility PRIVATE ${WARNING_COMPILE_FLAGS})
# Utility end -----------------------------------------------------------------------------------------------------------------------------

# ------------------------ Test -----------------------------------------------------------------------------------------------------------
add_library(Test
source/Test/MemoryCorrectnessItem.hpp
source/Test/MemoryCorrectnessItem.cpp
source/Test/TestManager.hpp
source/Test/TestManager.cpp
source/Test/Tests/ECSTester.hpp
source/Test/Tests/ECSTester.cpp
source/Test/Tests/ResourceManagerTester.hpp
source/Test/Tests/ResourceManagerTester.cpp
source/Test/Tests/GeometryTester.hpp
source/Test/Tests/GeometryTester.cpp
)
target_include_directories(Test
PRIVATE source/Test/Tests
PRIVATE source/Test
PRIVATE source
)
target_link_libraries(Test
PUBLIC Utility
PRIVATE ECS
PRIVATE Geometry
PRIVATE GLM
PRIVATE ImGui
PRIVATE OpenGL
)
target_compile_options(ECS PRIVATE ${WARNING_COMPILE_FLAGS})
# Test end --------------------------------------------------------------------------------------------------------------------------------

# EXTERNAL LIBRARIES ***********************************************************************************************************************
	# ImGui --------------------------------------------------------------------------------------------------------------------------------
	# ImGui has no cMakeLists.txt file so need to generate a library for it
	add_library(ImGui
	source/External/ImGui/imgui_demo.cpp
	source/External/ImGui/imgui_draw.cpp
	source/External/ImGui/imgui_tables.cpp
	source/External/ImGui/imgui_widgets.cpp
	source/External/ImGui/imgui.cpp
	source/External/ImGui/backends/imgui_impl_opengl3.cpp
	source/External/ImGui/backends/imgui_impl_glfw.cpp
	source/External/ImGuizmo/ImGuizmo.h
	source/External/ImGuizmo/ImGuizmo.cpp
	source/External/ImGuiUser/imgui_user.h
	source/External/ImGuiUser/imgui_user_config.h
	)
	target_include_directories(ImGui SYSTEM
	PUBLIC source/External/ImGui
	PUBLIC source/External/ImGuizmo
	PUBLIC source/External/ImGuiUser # Seperate folder to avoid comitting to ImGui
	PRIVATE source/External/GLFW/include
	)
	target_compile_definitions(ImGui
	INTERFACE IMGUI_USER_CONFIG="imgui_user_config.h")

	target_link_libraries(ImGui
	PUBLIC GLM
	PUBLIC Utility # imgui_user_config uses logger
	)
	# ImGui end ---------------------------------------------------------------------------------------------------------------------------

	# GLFW --------------------------------------------------------------------------------------------------------------------------------
	# Dont build any examples, tests or documentation
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	add_subdirectory(source/External/GLFW source/External/GLFW SYSTEM)
	# GLFW end ----------------------------------------------------------------------------------------------------------------------------

	# GLAD --------------------------------------------------------------------------------------------------------------------------------
	add_subdirectory(source/External/GLAD)
	# GLAD end ----------------------------------------------------------------------------------------------------------------------------

	# GLM ---------------------------------------------------------------------------------------------------------------------------------
	add_library(GLM INTERFACE)
	target_include_directories(GLM SYSTEM
	INTERFACE source/External/GLM)
	# GLM end -----------------------------------------------------------------------------------------------------------------------------

	# STB ---------------------------------------------------------------------------------------------------------------------------------
	add_library(STB INTERFACE)
	target_include_directories(STB SYSTEM
	INTERFACE source/External/stb)
	# STB end -----------------------------------------------------------------------------------------------------------------------------

	# ASSIMP ------------------------------------------------------------------------------------------------------------------------------
	set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
	if (WIN32)
		# Remove all supported filetypes for import and export.
		# Enable only OBJ files for import.
		set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
		set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
		set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
		set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	else()
		set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
	endif (WIN32)

	add_subdirectory(source/External/ASSIMP source/External/ASSIMP SYSTEM)
	# ASSIMP end --------------------------------------------------------------------------------------------------------------------------
# EXTERNAL LIBRARIES ***********************************************************************************************************************
