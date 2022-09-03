#ifndef PATHFINDER_GPU_PLATFORM_H
#define PATHFINDER_GPU_PLATFORM_H

#include "driver.h"

namespace Pathfinder {
    /// GLFW platform.
    class Platform {
    public:
        static std::shared_ptr<Platform> new_impl(DeviceType device_type,
                                                  uint32_t window_width,
                                                  uint32_t window_height);

        virtual std::shared_ptr<Driver> create_driver() = 0;

        virtual std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver> &driver,
                                                             uint32_t window_width,
                                                             uint32_t window_height) = 0;

        virtual void cleanup() = 0;

#ifndef __ANDROID__
    public:
        inline GLFWwindow *get_glfw_window() const {
            return window;
        }

        /// Process input events: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
        inline void poll_events() {
            glfwPollEvents();

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }
        }

    protected:
        GLFWwindow *window{};
#endif
    };
}

#endif //PATHFINDER_GPU_PLATFORM_H
