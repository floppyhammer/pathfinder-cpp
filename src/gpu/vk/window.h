#ifndef PATHFINDER_GPU_WINDOW_VK_H
#define PATHFINDER_GPU_WINDOW_VK_H

#include <cstring>
#include <iostream>
#include <vector>

#include "../../common/global_macros.h"
#include "../window.h"
#include "device.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class WindowVk : public Window {
    friend class WindowBuilderVk;

public:
    explicit WindowVk(const Vec2I &_size, GLFWwindow *window_handle, VkSurfaceKHR surface, VkInstance instance);

    ~WindowVk() override;

    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const;

    std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Device> &device) override;

public:
    VkSurfaceKHR surface_{};

    VkInstance instance_{};

private:
    void destroy();
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_WINDOW_VK_H
