# Meltdown Engine

## Description

**Meltdown Engine** is a cross platform graphics engine, using the **Vulkan** API for rendering, and is written in C++.
The engine aims to be lightweight and to achieve high performance.

Intended features for this project include:
- Render models using the Vulkan API;
- Load textures, models and scenes from external files;
- Simulate physics;
- Interact with the environment;
- Manipulate lightning;



## Install Guide

Required tools to install and execute the **Meltdown Engine** in a development environment:
- [CMake](https://cmake.org) (version 3.26 or higher), as well as a [generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html), to compile the code;
- [vcpkg](https://vcpkg.io) to download required libraries (optionally, the dependencies can be installed manually);
- [Vulkan SDK](https://vulkan.lunarg.com) to use Vulkan features required by the engine;

After downloading and installing the required tools, find the file `/scripts/buildsystems/vcpkg.cmake` at the **vcpkg** install location, and set the environment variable `CMAKE_TOOLCHAIN_FILE` to this file full path.


Make sure **CMake**, **vcpkg** and the **Vulkan SDK** executables are accessible through the command line:

```bash
cmake --version

vcpkg --version

glslc --version
```

If not, add the executables to the PATH.


The required libraries can be installed by running the following command:

```bash
vcpkg install glfw3 glm vulkan nlohmann-json imgui[core,glfw-binding,vulkan-binding]
```


To build the project, run the command:

```bash
cmake -S . -B build/ -G <generator_of_your_choice>
```

Use the chosen generator to compile the code and create the executable. This step will be different, depending on the generator used.
