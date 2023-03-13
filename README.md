# Pathfinder C++

This is a C++ port of [Pathfinder 3](https://github.com/servo/pathfinder) with a Vulkan backend added.

## Requirements

* `D3D9` Vulkan / OpenGL 3.3+ / OpenGL ES 3.0+.
* `D3D11` Vulkan / OpenGL 4.3+ / OpenGL ES 3.1+.

## Run demos

### Shader generation

* Run `src/shaders/compile_and_convert.ps1` to generate embedded shader headers. You should have Vulkan SDK installed
  first.

### Native

* Pull submodules.
* Load CMake project `demo/native/CMakeList.txt`.
* Build and run.

### Android

* Run `ndk/compile_and_copy.py` to build the static library and copy the header files.
* Open `demo/android-gles` or `demo/android-vulkan` in Android Studio.
* Build and run.

### Web (for Windows)

1. Change shader version to `#version 300 es` in all related shaders.

2. Regenerate shaders.

3. Have the emscripten environment set up.

4. Convert the CMake project to a makefile project using CMake GUI.
    * Locate the source code.
    * Configurate.
    * Specify `MinGW Makefiles` as the generator.
    * Specify toolchain file for cross-compiling.
      `[path_to_your_emsdk]/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake`
    * Generate.

5. Go to the folder containing the generated makefile.
   `[path_to_your_emsdk]/emsdk/emsdk activate latest`
   `[path_to_your_emsdk]/emsdk/upstream/emscripten/emmake make`
