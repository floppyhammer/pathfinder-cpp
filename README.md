# Pathfinder C++

This is a C++ port of [Pathfinder 3](https://github.com/servo/pathfinder) with a Vulkan backend added.

## Requirements

* C++14.
* D3D9 level: `Vulkan` / `OpenGL >=3.3` / `OpenGL ES >=3.0` / `WebGL2`.
* D3D11 level: `Vulkan` / `OpenGL >=4.3` / `OpenGL ES >=3.1`.

Noteable: D3D11 level is quite limited in OpenGL ES for it not being able to do both image read & write in a single
compute
invocation.

## Run demo

### Shader generation

* Run `src/shaders/compile_and_convert.ps1` to generate embedded shader headers. You should have Vulkan SDK installed
  first.

### Windows / Linux

* Pull submodules.
* Load CMake project `demo/native/CMakeList.txt`.
* Build and run.

### Android

* Copy `assets` into `demo/android-gles/app/src/main` or `demo/android-vulkan/app/src/main`
* Open `demo/android-gles` or `demo/android-vulkan` in Android Studio.
* Build and run.

### Web

* Have your emscripten environment set up.
* Run `wasm/build.ps1` to build the demo.
* Run `demo/web/serve.ps1` to serve a local website.
* Open http://127.0.0.1:8000/.
