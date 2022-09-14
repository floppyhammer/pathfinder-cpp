# Pathfinder C++

This is a C++ port of [Pathfinder 3](https://github.com/servo/pathfinder) with a Vulkan backend added.

## Requirements

* `D3D9` Vulkan / OpenGL 3.3+ / OpenGL ES 3.0+.

* `D3D11` Vulkan / OpenGL 4.3+ / OpenGL ES 3.1+.

## How to use

### Native

* Load `demo/native/CMakeList.txt` to run the native demo.

### Android

* Run `ndk/compile_and_copy.ps1` to build the shared library and copy the header files.

* Open `demo/android` in Android Studio to run the Android demo.
