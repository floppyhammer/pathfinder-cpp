# Pathfinder C++

This is a C++ port of [Pathfinder 3](https://github.com/servo/pathfinder) with a Vulkan backend added.

## Requirements

* C++14.
* D3d9 mode: `Vulkan` / `OpenGL 3.3 (or higher)` / `OpenGL ES 3.0 (or higher)` / `WebGL2`.
* D3d11 mode: `Vulkan` / `OpenGL 4.3 (or higher)` / `OpenGL ES 3.1 (or higher)`.

Notable: The D3d11 mode has some limits in an OpenGL ES context because, for ES, it's impossible to read & write
the same image in a single compute invocation.

## Run demo

### Shader generation (Optional)

* Run `src/shaders/compile_and_convert.ps1` to generate embedded shader headers. You should have Vulkan SDK installed
  first.

### Windows / Linux / Mac

* Initialize submodules.
* Build and run the CMake project `demo/native/CMakeList.txt`.

### Android

* Copy `assets` into `demo/android/app/src/main`.
* Open `demo/android` in Android Studio.
* Build and run.

### Web

* Set up emscripten environment.
* Run `build_wasm.sh` to build the demo.
* Run `demo/web/serve.sh` to serve a local website.
* Open http://127.0.0.1:8000/.
