project(Input)
add_library(${PROJECT_NAME} Input.hpp Input.cpp )

target_include_directories(${PROJECT_NAME} 
PRIVATE ../Logger
PRIVATE ../Renderer/GraphicsContext
)

target_link_directories(${PROJECT_NAME} 
PRIVATE ../Logger
PRIVATE ../Renderer/GraphicsContext
)

target_link_libraries(${PROJECT_NAME} 
Logger
GraphicsContext
)