project(GraphicsContext)
add_library(${PROJECT_NAME} Context.hpp OpenGLContext.hpp OpenGLContext.cpp )

glad_add_library(glad_gl REPRODUCIBLE MX API gl:core=3.3)

target_include_directories(${PROJECT_NAME} 
PRIVATE ../../../build/GraphicsContext/gladsources/glad_gl/include
PRIVATE ../../external/GLFW/include
PRIVATE ../../Logger
)

target_link_directories(${PROJECT_NAME} 
PRIVATE ../../../build/GraphicsContext/gladsources/glad_gl/src
PRIVATE ../../external/GLFW/src
PRIVATE ../../Logger
)

target_link_libraries(${PROJECT_NAME} 
glad_gl
glfw
Logger
)