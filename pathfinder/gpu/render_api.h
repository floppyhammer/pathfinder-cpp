#ifndef PATHFINDER_NATIVE_RENDER_API_H
#define PATHFINDER_NATIVE_RENDER_API_H

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
        // Include OpenGL header via GLAD.
        #include <glad/gl.h>
        #define GLFW_INCLUDE_NONE
    #endif

    #ifdef PATHFINDER_USE_VULKAN
        #define GLFW_INCLUDE_VULKAN
    #endif

    #include <GLFW/glfw3.h>

#endif

#endif // PATHFINDER_NATIVE_RENDER_API_H
