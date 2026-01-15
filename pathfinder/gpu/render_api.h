#ifndef PATHFINDER_NATIVE_RENDER_API_H
#define PATHFINDER_NATIVE_RENDER_API_H

#include "../config.h"

#ifdef __ANDROID__
    #ifdef PATHFINDER_USE_OPENGL

        #ifdef PATHFINDER_ENABLE_D3D11
            #include <GLES3/gl31.h>
        #else
            #include <GLES3/gl3.h>
        #endif

    #endif

    #ifdef PATHFINDER_USE_VULKAN
        // Vulkan header.
        #include "vulkan_wrapper.h"
    #endif

#elif defined(__EMSCRIPTEN__)
    #define GLFW_INCLUDE_ES3
    #include <GLFW/glfw3.h>
#else // Native

    #ifdef PATHFINDER_USE_OPENGL
    #if defined(__linux__) && defined(__ARM_ARCH)
        // Include OpenGL ES header via GLAD.
        #include <glad/gles2.h>
    #else
        // Include OpenGL header via GLAD.
        #include <glad/gl.h>
    #endif
    #define GLFW_INCLUDE_NONE
    #endif

    #ifdef PATHFINDER_USE_VULKAN
        #define GLFW_INCLUDE_VULKAN
        #include <vulkan/vulkan.h>
    #endif

    #include <GLFW/glfw3.h>

#endif

#endif // PATHFINDER_NATIVE_RENDER_API_H
