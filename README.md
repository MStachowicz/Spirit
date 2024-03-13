[![Build](https://github.com/MStachowicz/Spirit/actions/workflows/build.yml/badge.svg)](https://github.com/MStachowicz/Spirit/actions/workflows/build.yml)

# Spirit 👻
Spirit is a 3D game engine targetting Linux and Windows. This is a hobby project under <sup>~~slow~~</sup>active development with the intention to provide an environment as close to raw code for making games.

## Requirements
[Git](https://git-scm.com/downloads) (Submodules)\
[CMake 3.22](https://cmake.org/download/)\
C++ 20 [compliant compiler](https://en.cppreference.com/w/cpp/compiler_support)
<sub>(GCC 13 | CLANG 14 | MSVC 16.10)<sub>

## Setup
1. Clone repository with submodules.\
``` git clone https://github.com/MStachowicz/Spirit.git --recursive ```
2. Generate
In terminal use:\
```cmake -B Build```
OR
Open in an editor supporting [CMakePresets.json](https://github.com/MStachowicz/Spirit/blob/master/CMakePresets.json)
3. Open ```Spirit.exe``` inside the build directory

## Dependencies
A list of the [submodules](https://github.com/MStachowicz/Spirit/blob/master/.gitmodules) used:\
[GLFW](https://github.com/glfw/glfw) - GL/GLES/Vulkan API for creating windows, contexts, reading input, handling events.\
[GLAD](https://github.com/kieranvs/glad) - GL/GLES/EGL/GLX/WGL Loader/Generator.\
[GLM](https://github.com/g-truc/glm.git) - Math library.\
[ImGui](https://github.com/ocornut/imgui) - Immediate mode data-centric UI to display the editing interface in debug builds.\
[STB](https://github.com/nothings/stb.git) - Image file loading utility.\
[ASSIMP](https://github.com/assimp/assimp.git) - Model file loading utility.

The single root [CMakeLists.txt](https://github.com/MStachowicz/Spirit/blob/master/source/CMakeLists.txt) is responsible for building all the dependencies.
