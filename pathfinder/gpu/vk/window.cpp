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
    glfw_window_ = window_handle;

    glfwSetWindowUserPointer(glfw_window_, this);
    glfwSetFramebufferSizeCallback(glfw_window_, framebuffer_resize_callback);
}
#else
WindowVk::WindowVk(const Vec2I& _size, VkSurfaceKHR surface, VkInstance instance) : Window(_size) {
    surface_ = surface;
    instance_ = instance;
}
#endif

#ifndef __ANDROID__
void WindowVk::framebuffer_resize_callback(GLFWwindow* glfw_window, int width, int height) {
    auto window = reinterpret_cast<WindowVk*>(glfwGetWindowUserPointer(glfw_window));

    if (window) {
        window->just_resized_ = true;
        window->size_ = {width, height};
        window->minimized_ = window->size_.area() == 0;

        Logger::info("Window resized to " + window->size_.to_string());
    } else {
        Logger::error("glfwGetWindowUserPointer is NULL!");
    }
}

void WindowVk::poll_events() {
    just_resized_ = false;

    glfwPollEvents();

    if (glfwGetKey(glfw_window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(glfw_window_, true);
    }
}

bool WindowVk::should_close() {
    return glfwWindowShouldClose(glfw_window_);
}

void* WindowVk::get_raw_handle() const {
    return glfw_window_;
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
        swapchain_ = std::make_shared<SwapChainVk>(size_, this, device_vk);
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
