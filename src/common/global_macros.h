#ifndef PATHFINDER_GLOBAL_MACROS_H
#define PATHFINDER_GLOBAL_MACROS_H

// Choose between D3D9 and D3D11.
//#define PATHFINDER_USE_D3D11

// Enable DEBUG mode. Influencing performance.
// For GL, we will check for errors.
// For Vulkan, we will enable validation layers.
#define PATHFINDER_DEBUG

// Enable building scene in parallel.
#define PATHFINDER_THREADS 4

// Enable SIMD.
#define PATHFINDER_ENABLE_SIMD

#endif // PATHFINDER_GLOBAL_MACROS_H
