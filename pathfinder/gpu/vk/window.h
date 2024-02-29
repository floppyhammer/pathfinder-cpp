#ifndef PATHFINDER_GPU_WINDOW_VK_H
#define PATHFINDER_GPU_WINDOW_VK_H

#include <cstring>
#include <iostream>
#include <vector>

#include "../../common/global_macros.h"
#include "../window.h"
#include "device.h"

namespace Pathfinder {

class WindowVk : public Window {
    friend class WindowBuilderVk;

public:
#ifndef __ANDROID__
    explicit WindowVk(const Vec2I &_size, GLFWwindow *window_handle, VkSurfaceKHR surface, VkInstance instance);
#else
    explicit WindowVk(const Vec2I &_size, VkSurfaceKHR surface, VkInstance instance);
#endif

    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const;

    std::shared_ptr<SwapChain> get_swap_chain(const std::shared_ptr<Device> &device) override;

    VkSurfaceKHR surface_{};

    VkInstance instance_{};

private:
    void destroy() override;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_WINDOW_VK_H
