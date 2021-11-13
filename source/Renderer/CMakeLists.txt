project(Renderer)
add_library(${PROJECT_NAME} Renderer.hpp Renderer.cpp )
add_subdirectory(GraphicsContext)

target_include_directories(${PROJECT_NAME} 
PRIVATE ../Logger
PRIVATE GraphicsContext
)

target_link_directories(${PROJECT_NAME} 
PRIVATE ../Logger
PRIVATE GraphicsContext
)

target_link_libraries(${PROJECT_NAME} 
Logger
GraphicsContext
)
