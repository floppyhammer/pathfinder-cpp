# Pathfinder C++

This is a C++ port of [Pathfinder 3](https://github.com/servo/pathfinder).

## Requirements

* `D3D9` OpenGL 3.3+, OpenGL ES 3.0+.

* `D3D11` OpenGL 4.3+, OpenGL ES 3.1+.

## How to use

### Native

* Load `demo/native/CMakeList.txt` to run the native demo.

### Android

* Run `ndk/compile_and_copy.ps1` to build the shared library and copy the header files.

* Open `demo/android` in Android Studio to run the Android demo.

### Changing shaders

* Every time you change shaders, use `src/shaders/minifier.py` to regenerate minified ones for shader embedding.
