#pragma once

#include "../window.h"
#include "device.h"

namespace Pathfinder {

class WindowVk : public Window {
    friend class WindowBuilderVk;

public:
    WindowVk(const Vec2I &_size, void *window_handle, VkSurfaceKHR surface, VkInstance instance);

    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const;

    std::shared_ptr<SwapChain> get_swap_chain(const std::shared_ptr<Device> &device, PresentMode present_mode) override;

    void destroy() override;

    VkSurfaceKHR surface_{};

    VkInstance instance_{};
};

} // namespace Pathfinder
