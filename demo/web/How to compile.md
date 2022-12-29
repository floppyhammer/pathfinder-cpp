# Method 1 (doesn't work, no wasm and js files generated)

D:
cd D:/pathfinder-cpp

D:/Env/wasm/emsdk/emsdk activate latest

mkdir build

cd build

D:/Env/wasm/emsdk/upstream/emscripten/emcmake cmake .. -G "MinGW Makefiles"

D:/Env/wasm/emsdk/upstream/emscripten/emmake make

# Method 2

Use CMake to create makefile.

Go to the folder containing the makefile and activate emsdk as above.

D:/Env/wasm/emsdk/upstream/emscripten/emmake make
