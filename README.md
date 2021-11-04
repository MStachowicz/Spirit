# Zephyr
The goal of Zephyr is to be a CMake generated cross-platform renderer/game engine which allows hot-swapping of APIs (OpenGL/Vulkan/DirectX).

## Requirements to run Zephyr
Git (Submodules)\
CMake (Generate... everything)\
Python3 (GLAD2)

## How to grab the source code
``` git clone https://github.com/MStachowicz/Zephyr.git --recursive ```

## How to generate
Open the root Zephyr folder in command line and call:\

```cmake -S Source -B Build```

Optionally specify your chosen generator using -G "<generator name>" e.g. -G "Visual Studio 17 2022"
  
## Dependencies
These are the external libraries/dependencies of zephyr:\
[GLFW](https://github.com/glfw/glfw) - Cross-platform GL/GLES/Vulkan API for creating windows, contexts, reading input, handling events\
[GLAD2](https://github.com/Dav1dde/glad/tree/glad2) - GL/GLES/EGL/GLX/WGL Loader/Generator based on the user graphics driver\
[spdlog](https://github.com/gabime/spdlog) - Multithreaded logging library
  
 All of these are added recursively using git submodules and generated using the top level [CMakeLists.txt](https://github.com/MStachowicz/Zephyr/blob/master/source/CMakeLists.txt)
