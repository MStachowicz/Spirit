project(Renderer)
add_library(${PROJECT_NAME} Renderer.hpp Renderer.cpp )

target_include_directories(${PROJECT_NAME} 
PRIVATE ../external/GLFW/include
)

target_link_directories(${PROJECT_NAME} 
PRIVATE ../external/GLFW/src
)

target_link_libraries(${PROJECT_NAME} 
glfw
)