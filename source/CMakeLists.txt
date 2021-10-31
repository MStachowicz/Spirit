cmake_minimum_required(VERSION 3.6)

project(Zephyr)
add_executable(${PROJECT_NAME} Zephyr.cpp)

add_subdirectory(External/GLFW) # Git Submodule GLFW
add_subdirectory(External/spdlog) # Git Submodule GLFW
add_subdirectory(Renderer)
add_subdirectory(Logger)

target_include_directories(${PROJECT_NAME} 
PRIVATE Renderer
PRIVATE Logger
)

target_link_directories(${PROJECT_NAME} 
PRIVATE Renderer
PRIVATE Logger
)

target_link_libraries(${PROJECT_NAME} 
Renderer
Logger
)

# Setting properties for SLN projects 
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}) # Make Zephyr the startup project
set(CMAKE_SUPPRESS_REGENERATION true) # Removes the ZERO_CHECK project from the build