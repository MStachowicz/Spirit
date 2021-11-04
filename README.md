The goal of Zephyr is to be a CMAKE generated cross-platform renderer/game engine which allows hot-swapping of APIs (OpenGL/Vulkan/DirectX).

Requirements to run Zephyr:
Git (Submodules)
CMake (Generate... everything)
Python3 (GLAD2)

How to grab source code:
git clone https://github.com/MStachowicz/Zephyr.git --recursive

How to generate:
Open the root Zephyr folder in command line and call:
  cmake -S Source -B Build
Optionally specify your chosen generator using -G "<generator name>" e.g. -G "Visual Studio 17 2022"
