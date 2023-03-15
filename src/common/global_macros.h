#ifndef PATHFINDER_GLOBAL_MACROS_H
#define PATHFINDER_GLOBAL_MACROS_H

// Choose between D3D9 and D3D11.
// #define PATHFINDER_USE_D3D11

// Enable this if GLES3.0 support is needed.
// #define PATHFINDER_MINIMUM_SHADER_VERSION_SUPPORT

// WebGL only supports ES3.0 shaders.
#ifdef __EMSCRIPTEN__
    #define PATHFINDER_MINIMUM_SHADER_VERSION_SUPPORT
#endif

// Enable DEBUG mode. Influencing performance.
// For GL, we will check for errors.
// For Vulkan, we will enable validation layers.
#define PATHFINDER_DEBUG

// Enable building scene in parallel.
#define PATHFINDER_THREADS 4

// Enable SIMD.
#ifndef __EMSCRIPTEN__
    #define PATHFINDER_ENABLE_SIMD
#endif

#endif // PATHFINDER_GLOBAL_MACROS_H
