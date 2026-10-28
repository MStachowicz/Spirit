# ![](source/Resources/Textures/Icons/64.png) Spirit

A 3D game engine targeting Linux and Windows. This is a hobby project evolving as I learn, build, and break things along the way.

[![Build](https://github.com/MStachowicz/Spirit/actions/workflows/build.yml/badge.svg)](https://github.com/MStachowicz/Spirit/actions/workflows/build.yml)
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
[GLFW 3.5.1](https://github.com/glfw/glfw) - GL/GLES/Vulkan API for creating windows, contexts, reading input, handling events.\
[GLAD 0.1.29](https://github.com/kieranvs/glad) - GL/GLES/EGL/GLX/WGL Loader/Generator.\
[GLM 1.0.3](https://github.com/g-truc/glm) - Math library.\
[ImGui 1.93.0-docking](https://github.com/ocornut/imgui) - Immediate mode data-centric UI to display the editing interface in debug builds.\
[ImGuizmo 1.92.5](https://github.com/CedricGuillemet/ImGuizmo) - Transform manipulation gizmos.\
[STB (stb_image 2.30, stb_image_write 1.16)](https://github.com/nothings/stb) - Image file loading utility.\
[Tracy 0.14.1](https://github.com/wolfpld/tracy) - Profiler.\
[Jolt Physics 5.6.0](https://github.com/jrouwe/JoltPhysics) - Rigid-body physics.

The single root [CMakeLists.txt](https://github.com/MStachowicz/Spirit/blob/master/source/CMakeLists.txt) is responsible for building all the dependencies.
