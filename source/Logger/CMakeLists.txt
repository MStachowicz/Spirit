project(Logger)
add_library(${PROJECT_NAME} Logger.hpp Logger.cpp )

target_include_directories(${PROJECT_NAME} 
PRIVATE ../external/spdlog/include
)

target_link_directories(${PROJECT_NAME} 
PRIVATE ../external/spdlog/src
)

target_link_libraries(${PROJECT_NAME} 
spdlog
)