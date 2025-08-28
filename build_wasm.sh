mkdir cmake-build-wasm
cd cmake-build-wasm || exit
emcmake cmake .. -DPATHFINDER_BUILD_DEMO=ON
emmake make
