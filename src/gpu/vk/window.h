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

    ~WindowVk() override;

    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const;

    std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Device> &device) override;

#ifndef __ANDROID__
public:
    GLFWwindow *get_glfw_window() const;

    /// GLFW: whenever the window size changed (by OS or user) this callback function executes.
    static void framebuffer_resize_callback(GLFWwindow *glfw_window, int width, int height);

    /// Process input events: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
    void poll_events() override;

    bool should_close() override;
#endif

public:
    VkSurfaceKHR surface_{};

    VkInstance instance_{};

private:
    void destroy();

#ifndef __ANDROID__
    GLFWwindow *glfw_window{};
#endif
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_WINDOW_VK_H
