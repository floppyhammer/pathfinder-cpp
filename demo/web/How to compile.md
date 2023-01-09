# Method 1 (doesn't work, no wasm and js files generated)

1. Go to the project folder.

2. Activate emsdk.
   `D:/Env/emsdk/emsdk activate latest`

3. Create a build folder.

   `mkdir build`

   `cd build`

4. CMake.

   `D:/Env/emsdk/upstream/emscripten/emcmake cmake .. -G "MinGW Makefiles"`

5. Make.

   `D:/Env/emsdk/upstream/emscripten/emmake make`

# Method 2

1. Use CMake to create makefile.

2. Specify toolchain file for cross-compiling.

   `D:/Env/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake`

3. Go to the folder containing the created makefile.

   `D:/Env/emsdk/emsdk activate latest`

   `D:/Env/emsdk/upstream/emscripten/emmake make`
