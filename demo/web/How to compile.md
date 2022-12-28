D:
cd D:/pathfinder-cpp

D:/Env/wasm/emsdk/emsdk activate latest

mkdir build

cd build

D:/Env/wasm/emsdk/upstream/emscripten/emcmake cmake .. -G "MinGW Makefiles"

D:/Env/wasm/emsdk/upstream/emscripten/emmake make
