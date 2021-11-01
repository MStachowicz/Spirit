project(Renderer)
add_library(${PROJECT_NAME} Renderer.hpp Renderer.cpp )

target_include_directories(${PROJECT_NAME} 
PRIVATE ../external/GLFW/include
PRIVATE ../Logger
)

target_link_directories(${PROJECT_NAME} 
PRIVATE ../external/GLFW/src
PRIVATE ../Logger
)

target_link_libraries(${PROJECT_NAME} 
glfw
Logger
)