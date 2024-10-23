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
- [CMake](https://cmake.org) (version 3.26 or higher), as well as a
	[generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html), to compile the code;
- [vcpkg](https://vcpkg.io) to download required libraries (optionally, the dependencies can be installed manually);
- [Vulkan SDK](https://vulkan.lunarg.com) to use Vulkan features required by the engine;

After downloading and installing the required tools, find the file `/scripts/buildsystems/vcpkg.cmake`
at the **vcpkg** install location, and set the environment variable `CMAKE_TOOLCHAIN_FILE` to this file full path.

---

Make sure **CMake**, **vcpkg** and the **Vulkan SDK** executables are accessible through the command line:

```bash
cmake --version

vcpkg --version

glslc --version
```

If they are not accessible, try adding the executables to the **PATH**.

---

The required libraries can be installed by running the following command:

```bash
vcpkg install glfw3 glm vulkan nlohmann-json stb imgui[docking-experimental,glfw-binding,vulkan-binding]
```

If on Windows, it is recommended to link the packages statically, to avoid missing DLL problems
when running an application that uses the Meltdown Engine.
This can be done by appending `--triplet=x64-windows-static` to the command above.

---

To build the project, run the command:

```bash
cmake -S . -B build/ -G <Generator of your choice> -D CMAKE_BUILD_TYPE=<Build type>
```

Use the chosen generator to compile the code and create the executable.
This step will be different, depending on the generator used.

The build type can be `Debug`, `Release`, `RelWithDebInfo` or `MinSizeRel`. If not defined, the
build type will be set to `Debug`.

On Windows, if the `x64-windows-static` triplet has been used, it is also necessary to append
`-D VCPKG_TARGET_TRIPLET=x64-windows-static` to the **cmake** command.


## Recommendations

Create scripts such as `build.sh`, `run.sh` and `clear.sh` (or `.cmd`/`.ps1` if on Windows) to
build and run the Engine more easily.
