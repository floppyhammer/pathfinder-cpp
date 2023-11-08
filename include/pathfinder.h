#ifndef PATHFINDER_API_OPENGL_H
#define PATHFINDER_API_OPENGL_H

#include "../src/common/global_macros.h"
#include "../src/common/io.h"
#include "../src/common/math/basic.h"
#include "../src/common/math/mat4.h"
#include "../src/common/math/rect.h"
#include "../src/common/math/vec2.h"
#include "../src/common/math/vec3.h"
#include "../src/common/timestamp.h"
#include "../src/core/canvas.h"
#include "../src/core/svg.h"
#include "../src/gpu/framebuffer.h"
#include "../src/gpu/swap_chain.h"
#ifdef PATHFINDER_USE_VULKAN
    #include "../src/gpu/vk/device.h"
    #include "../src/gpu/vk/queue.h"
    #include "../src/gpu/vk/window.h"
    #include "../src/gpu/vk/window_builder.h"
#else
    #include "../src/gpu/gl/device.h"
    #include "../src/gpu/gl/queue.h"
    #include "../src/gpu/gl/window.h"
    #include "../src/gpu/gl/window_builder.h"
#endif

#endif // PATHFINDER_API_OPENGL_H
