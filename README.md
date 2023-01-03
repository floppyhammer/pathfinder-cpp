# Pathfinder C++

This is a C++ port of [Pathfinder 3](https://github.com/servo/pathfinder) with a Vulkan backend added.

## Requirements

* `D3D9` Vulkan / OpenGL 3.3+ / OpenGL ES 3.0+.
* `D3D11` Vulkan / OpenGL 4.3+ / OpenGL ES 3.1+.

## How to run demos

### Shader generation

* Run `src/shaders/compile_and_convert.ps1` to generate shader headers. You should have Vulkan SDK installed first.

### Native

* Load `demo/native/CMakeList.txt` to run the native demo.

### Android

* Run `ndk/compile_and_copy.py` to build the static library and copy the header files.
* Open `demo/android-gles` or `demo/android-vulkan` in Android Studio to run the Android demo.

### Web

* Change shader version to `#version 300 es` in all related shaders.
* Regenerate shaders.
* Use CMake to generate a Wasm project.
* Compile .wasm file.
