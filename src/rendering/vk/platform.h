#ifndef PATHFINDER_PLATFORM_VK_H
#define PATHFINDER_PLATFORM_VK_H

#include "../vertex_input.h"
#include "../buffer.h"
#include "../texture.h"
#include "../command_buffer.h"
#include "../../common/io.h"
#include "../../common/global_macros.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class PlatformVk {
    public:
        GLFWwindow *window;

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;

        VkSurfaceKHR surface;

        /// The graphics card that we'll end up selecting will be stored in a VkPhysicalDevice handle.
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        /// Logical device.
        VkDevice device{};
    };
}

#endif

#endif //PATHFINDER_PLATFORM_VK_H
