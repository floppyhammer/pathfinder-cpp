# Pathfinder C++

This is a C++ port of [Pathfinder 3](https://github.com/servo/pathfinder) with a Vulkan backend added.

## Requirements

* C++14.
* Render workflow: `Vulkan` / `OpenGL 3.3 (or higher)` / `OpenGL ES 3.0 (or higher)` / `WebGL2`.
* Compute workflow: `Vulkan` / `OpenGL 4.3 (or higher)` / `OpenGL ES 3.1 (or higher)`.

Notable: The compute workflow has some limits in an OpenGL ES context because it's impossible to read & write
the same image in a single compute invocation.

## Run demo

### Shader generation

* Run `src/shaders/compile_and_convert.ps1` to generate embedded shader headers. You should have Vulkan SDK installed
  first.

### Windows / Linux / Mac

* Initialize submodules.
* Build and run the CMake project `demo/native/CMakeList.txt`.

### Android

* Copy `assets` into `demo/android-gles/app/src/main` or `demo/android-vulkan/app/src/main`
* Open `demo/android-gles` or `demo/android-vulkan` in Android Studio.
* Build and run.

### Web

* Set up emscripten environment.
* Run `wasm/build.ps1` to build the demo.
* Run `demo/web/serve.ps1` to serve a local website.
* Open http://127.0.0.1:8000/.
