//
// Created by chy on 8/3/2021.
//

#ifndef PATHFINDER_GLOBAL_MACROS_H
#define PATHFINDER_GLOBAL_MACROS_H

// Choose between D3D9 and D3D11.
//#define PATHFINDER_USE_D3D11

// Enable DEBUG mode, in which we will check for GL errors. Influencing performance.
//#define PATHFINDER_DEBUG

// This is optional for the native demo, as we can access shader files directly.
#define PATHFINDER_SHIP_SHADERS

// Enable building scene in parallel.
#define PATHFINDER_OPENMP_THREADS 4

// Enable this when integrated in VT2D.
//#define PATHFINDER_INTEGRATE_IN_VT2D

#define PATHFINDER_SHADER_DIR "../src/shaders/"
#define PATHFINDER_RES_DIR "../res/"

// Check bits for MSVC.
#if _WIN32 || _WIN64
#if _WIN64
#define PATHFINDER_64BIT
#else
#define PATHFINDER_32BIT
#endif
#endif

// Check bits for GCC.
#if __GNUC__
#if __x86_64__ || __ppc64__
#define PATHFINDER_64BIT
#else
#define PATHFINDER_32BIT
#endif
#endif

// Disable SIMD for x86 build.
#ifdef PATHFINDER_64BIT
#define PATHFINDER_SIMD_ENABLED
#endif

#ifdef __ANDROID__
#ifdef PATHFINDER_USE_D3D11
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif
#else
#ifdef PATHFINDER_INTEGRATE_IN_VT2D
#include "vtglcommon.hpp"
#else

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#endif
#endif

#endif //PATHFINDER_GLOBAL_MACROS_H
