//
// Created by floppyhammer on 8/3/2021.
//

#ifndef PATHFINDER_GLOBAL_MACROS_H
#define PATHFINDER_GLOBAL_MACROS_H

// Choose between D3D9 and D3D11.
//#define PATHFINDER_USE_D3D11

// Enable DEBUG mode, in which we will check for GL errors. Influencing performance.
//#define PATHFINDER_DEBUG

// Enable building scene in parallel.
#define PATHFINDER_OPENMP_THREADS 4

// Enable SIMD.
#define PATHFINDER_SIMD_ENABLED

// This is optional for the native demo, as we can access shader files directly.
#define PATHFINDER_SHIP_SHADERS
#define PATHFINDER_SHADER_DIR "../src/shaders/"
#define PATHFINDER_RES_DIR "../res/"

// OpenGL headers.
#ifdef __ANDROID__
#ifdef PATHFINDER_USE_D3D11
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif
#else
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#endif //PATHFINDER_GLOBAL_MACROS_H
