# Pathfinder C++

This is a C++ port of [Pathfinder 3](https://github.com/servo/pathfinder) with a Vulkan backend added.

## Requirements

* C++ 14 Standard.
* `D3D9` Vulkan / OpenGL 3.3+ / OpenGL ES 3.0+ / WebGL 2.
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

### Web

1. Have the emscripten environment set up.

2. Run `wasm/make.ps1` to build the demo.

3. Run `demo/web/serve.ps1` to serve a local website.

4. Open http://127.0.0.1:8000/.
