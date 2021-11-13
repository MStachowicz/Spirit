cmake_minimum_required(VERSION 3.6)
add_compile_options("$<$<CONFIG:DEBUG>:-DZEPHYR_CONFIG_DEBUG>$<$<CONFIG:RELEASE>:-DZEPHYR_CONFIG_RELEASE>")

project(Zephyr)

add_subdirectory(source)
add_subdirectory(source/External/GLFW) # Git Submodule GLFW
add_subdirectory(source/External/spdlog) # Git Submodule spdlog
add_subdirectory(source/External/GLAD2/cmake) # Git Submodule GLAD2
add_subdirectory(source/Renderer)
add_subdirectory(source/Logger)
add_subdirectory(source/Input)