mkdir cmake_build_wasm
cd cmake_build_wasm || exit
emcmake cmake .. -DPATHFINDER_BUILD_DEMO=ON
emmake make
