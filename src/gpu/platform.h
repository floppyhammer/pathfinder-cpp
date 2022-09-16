#ifndef PATHFINDER_GPU_PLATFORM_H
#define PATHFINDER_GPU_PLATFORM_H

#include "driver.h"

namespace Pathfinder {
    class Platform {
    public:
        Platform(uint32_t window_width, uint32_t window_height) : width(window_width), height(window_height) {}

        static std::shared_ptr<Platform> new_impl(DeviceType device_type,
                                                  uint32_t window_width,
                                                  uint32_t window_height);

        virtual std::shared_ptr<Driver> create_driver() = 0;

        virtual std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver> &driver) = 0;

        virtual void cleanup() = 0;

    public:
        bool framebuffer_resized = false;

    protected:
        uint32_t width;
        uint32_t height;

#ifndef __ANDROID__
    public:
        inline GLFWwindow *get_glfw_window() const {
            return window;
        }

        /// GLFW: whenever the window size changed (by OS or user) this callback function executes.
        static void framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
            auto platform = reinterpret_cast<Platform *>(glfwGetWindowUserPointer(window));
            platform->framebuffer_resized = true;
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
