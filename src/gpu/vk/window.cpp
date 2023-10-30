#include "window.h"

#include <set>
#include <stdexcept>

#include "debug_marker.h"
#include "device.h"
#include "queue.h"
#include "swap_chain.h"

namespace Pathfinder {

WindowVk::WindowVk(const Vec2I& _size, GLFWwindow* window_handle, VkSurfaceKHR surface, VkInstance instance)
    : Window(_size) {
    _surface = surface;
    _instance = instance;
    glfw_window = window_handle;
}

VkExtent2D WindowVk::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) const {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(glfw_window, &width, &height);

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
    if (_surface) {
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        _surface = nullptr;
    }

    if (glfw_window) {
        glfwDestroyWindow(glfw_window);
        glfw_window = nullptr;
    }
}

} // namespace Pathfinder
