add_executable(${PROJECT_NAME} Zephyr.cpp)

target_include_directories(${PROJECT_NAME} 
PRIVATE Renderer
PRIVATE Input
PRIVATE Logger
)

target_link_directories(${PROJECT_NAME} 
PRIVATE Renderer
PRIVATE Input
PRIVATE Logger
)

target_link_libraries(${PROJECT_NAME} 
Renderer
Input
Logger
)