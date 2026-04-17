---
description: "Use when building, compiling, linking, or running CMake targets. Covers build commands, compilation errors, and test execution."
applyTo: ["CMakeLists.txt", "CMakePresets.json", "**/*.cpp", "**/*.hpp"]
---
# Build Instructions

- **Never** run `cmake --build`, `cmake`, `make`, `ninja`, or `ctest` via the terminal. The terminal does not have the VS Developer Shell environment, so these commands will fail with missing standard library headers.
- **Always** use the CMake Tools extension to build: `Build_CMakeTools`, `ListBuildTargets_CMakeTools`, `RunCtest_CMakeTools`.
- When the user asks to build, compile, or fix build errors, use `Build_CMakeTools` with the appropriate target (e.g., `Spirit`).
- When the user asks to run tests, use `RunCtest_CMakeTools`.
