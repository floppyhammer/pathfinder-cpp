#ifndef PATHFINDER_GPU_WINDOW_VK_H
#define PATHFINDER_GPU_WINDOW_VK_H

#include <cstring>
#include <iostream>
#include <vector>

#include "../../common/global_macros.h"
#include "../window.h"
#include "device.h"

#ifdef PATHFINDER_USE_VULKAN

    // We don't need a Window on Android and Web.
    #if (defined(WIN32) || defined(__linux__) || defined(__APPLE__)) && !defined(ANDROID)

namespace Pathfinder {

class WindowVk : public Window {
    friend class WindowBuilderVk;

public:
    explicit WindowVk(const Vec2I &_size, GLFWwindow *window_handle, VkSurfaceKHR surface);

    ~WindowVk() override = default;

    VkQueue get_present_queue() const;

    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const;

    std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Device> &device) override;

public:
    VkSurfaceKHR surface{};

    VkInstance instance{};

    VkQueue present_queue{};
};

} // namespace Pathfinder

    #endif

#endif

#endif // PATHFINDER_GPU_WINDOW_VK_H
