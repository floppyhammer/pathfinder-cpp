#ifndef PATHFINDER_GLOBAL_MACROS_H
#define PATHFINDER_GLOBAL_MACROS_H

// Enable Dx11 render level.
#define PATHFINDER_ENABLE_D3D11

// Dx11 level is not available in Web.
#if defined(__EMSCRIPTEN__)
    #undef PATHFINDER_ENABLE_D3D11
#endif

// Enable this if ES3.0 support is needed.
// #define PATHFINDER_MINIMUM_SHADER_VERSION_SUPPORT

// WebGL only supports ES3.0 shaders.
#if defined(__EMSCRIPTEN__) || defined(ANDROID)
    #define PATHFINDER_MINIMUM_SHADER_VERSION_SUPPORT
#endif

// Enable DEBUG mode. Influencing performance.
// For GL, we will check for errors.
// For Vulkan, we will enable validation layers.
#define PATHFINDER_DEBUG

// Enable building scene in parallel.
#define PATHFINDER_THREADS 4

// Enable SIMD.
#if !defined(__EMSCRIPTEN__) && !defined(__APPLE__)
    #define PATHFINDER_ENABLE_SIMD
#endif

#endif // PATHFINDER_GLOBAL_MACROS_H
