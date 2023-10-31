#include "window.h"

#include <set>
#include <stdexcept>

#include "debug_marker.h"
#include "device.h"
#include "queue.h"
#include "swap_chain.h"

namespace Pathfinder {

#ifndef __ANDROID__
WindowVk::WindowVk(const Vec2I& _size, GLFWwindow* window_handle, VkSurfaceKHR surface, VkInstance instance)
    : Window(_size) {
    surface_ = surface;
    instance_ = instance;
    glfw_window = window_handle;
}
#else
    WindowVk::WindowVk(const Vec2I& _size, VkSurfaceKHR surface, VkInstance instance)
            : Window(_size) {
        surface_ = surface;
        instance_ = instance;
    }
#endif

VkExtent2D WindowVk::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) const {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
#ifndef __ANDROID__

        glfwGetFramebufferSize(glfw_window, &width, &height);
#endif
        width = 1000;
        height = 1000;

        VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actual_extent.width = std::max(capabilities.minImageExtent.width,
                                       std::min(capabilities.maxImageExtent.width, actual_extent.width));
        actual_extent.height = std::max(capabilities.minImageExtent.height,
                                        std::min(capabilities.maxImageExtent.height, actual_extent.height));

        return actual_extent;
    }
}

std::shared_ptr<SwapChain> WindowVk::create_swap_chain(const std::shared_ptr<Device>& device) {
    auto device_vk = static_cast<DeviceVk*>(device.get());
    return std::make_shared<SwapChainVk>(size, this, device_vk);
}

WindowVk::~WindowVk() {
    destroy();
}

void WindowVk::destroy() {
#ifndef __ANDROID__
    if (surface_) {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        surface_ = nullptr;
    }

    if (glfw_window) {
        glfwDestroyWindow(glfw_window);
        glfw_window = nullptr;
    }
#endif
}

} // namespace Pathfinder
