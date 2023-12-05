#ifndef PATHFINDER_GLOBAL_MACROS_H
#define PATHFINDER_GLOBAL_MACROS_H

/// Platform marcos.
#ifdef __EMSCRIPTEN__
    #define PATHFINDER_EMSCRIPTEN
#elif defined(_WIN32)
    #define PATHFINDER_WINDOWS
#elif defined(__linux__) && !defined(__ANDROID__)
    #define PATHFINDER_LINUX
#elif defined(__ANDROID__)
    #define PATHFINDER_ANDROID
#elif defined(__APPLE__)
    #define PATHFINDER_APPLE

    #import <TargetConditionals.h>
    #if TARGET_OS_OSX
        #define PATHFINDER_MACOS
    #elif TARGET_OS_IPHONE
        #define PATHFINDER_IOS
    #endif
#endif

// Enable Dx11 render level.
#define PATHFINDER_ENABLE_D3D11

// Dx11 level is not available in Web.
#ifdef PATHFINDER_EMSCRIPTEN
    #undef PATHFINDER_ENABLE_D3D11
#endif

// Enable this if ES3.0 support is needed.
// #define PATHFINDER_MINIMUM_SHADER_VERSION_SUPPORT

// WebGL only supports ES3.0 shaders.
#if defined(PATHFINDER_EMSCRIPTEN) || defined(PATHFINDER_ANDROID)
    #define PATHFINDER_MINIMUM_SHADER_VERSION_SUPPORT
#endif

// Enable DEBUG mode. Influencing performance.
// For the GL backend, error checking is enabled.
// For Vulkan , the validation layers are enabled.
#define PATHFINDER_DEBUG

/// Enable building scenes (dx9) in parallel.
#define PATHFINDER_THREADS 4

/// Enable SIMD.
#if !defined(PATHFINDER_EMSCRIPTEN) && !defined(PATHFINDER_APPLE)
    #define PATHFINDER_ENABLE_SIMD
#endif

#endif // PATHFINDER_GLOBAL_MACROS_H
