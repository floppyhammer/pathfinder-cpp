#include "window.h"

#include <set>
#include <stdexcept>

#include "debug_marker.h"
#include "device.h"
#include "queue.h"
#include "swap_chain.h"

namespace Pathfinder {

#ifndef __ANDROID__
WindowVk::WindowVk(const Vec2I& size, GLFWwindow* window_handle, VkSurfaceKHR surface, VkInstance instance)
    : Window(size, window_handle) {
    surface_ = surface;
    instance_ = instance;
}
#else
WindowVk::WindowVk(const Vec2I& _size, VkSurfaceKHR surface, VkInstance instance) : Window(_size) {
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

        glfwGetFramebufferSize(glfw_window_, &width, &height);
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

std::shared_ptr<SwapChain> WindowVk::get_swap_chain(const std::shared_ptr<Device>& device) {
    if (!swapchain_) {
        auto device_vk = static_cast<DeviceVk*>(device.get());
        swapchain_ = std::make_shared<SwapChainVk>(logical_size_, this, device_vk);
    }

    return swapchain_;
}

void WindowVk::destroy() {
#ifndef __ANDROID__
    if (surface_) {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        surface_ = nullptr;
    }

    if (glfw_window_) {
        glfwDestroyWindow(glfw_window_);
        glfw_window_ = nullptr;
    }
#endif
}

} // namespace Pathfinder
