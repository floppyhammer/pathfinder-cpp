#ifndef PATHFINDER_GPU_PLATFORM_H
#define PATHFINDER_GPU_PLATFORM_H

#include "gl/driver.h"
#include "vk/driver.h"

namespace Pathfinder {
    class Platform {
    public:
        virtual std::shared_ptr<Driver> create_driver() = 0;

        virtual std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver>& driver, uint32_t p_width, uint32_t p_height) = 0;

        inline GLFWwindow *get_glfw_window() const {
            return window;
        }

        /// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
        inline void poll_events() {
            glfwPollEvents();

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);
        }

        virtual void cleanup() = 0;

    protected:
        GLFWwindow *window;
    };
}

#endif //PATHFINDER_GPU_PLATFORM_H
