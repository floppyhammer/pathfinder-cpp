#include "window.h"

#include <sstream>
#include <stdexcept>

#include "../../common/logger.h"
#include "swap_chain.h"

#ifndef PATHFINDER_USE_VULKAN

    #if (defined(WIN32) || defined(__linux__))
        #include <glad/gl.h>

namespace Pathfinder {

WindowGl::WindowGl(const Vec2I &_size, GLFWwindow *window_handle) : Window(_size) {
    glfw_window = window_handle;

    // Assign this to window user, so we can fetch it when window size changes.
    glfwSetWindowUserPointer(glfw_window, this);
    glfwSetFramebufferSizeCallback(glfw_window, framebuffer_resize_callback);
}

std::shared_ptr<SwapChain> WindowGl::create_swap_chain(const std::shared_ptr<Device> &device) {
    return std::make_shared<SwapChainGl>(size, glfw_window);
}

} // namespace Pathfinder

    #endif

#endif
