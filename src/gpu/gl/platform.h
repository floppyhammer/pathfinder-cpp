#ifndef PATHFINDER_GPU_PLATFORM_GL_H
#define PATHFINDER_GPU_PLATFORM_GL_H

#include "../platform.h"
#include "../../common/global_macros.h"

#include <vector>
#include <iostream>
#include <optional>

namespace Pathfinder {
    class PlatformGl : public Platform {
    public:
        static PlatformGl &get_singleton() {
            static PlatformGl singleton;
            return singleton;
        }

        void init(uint32_t p_width, uint32_t p_height);

        /// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
        void handle_inputs();

        void swap_buffers_and_poll_events() const;

        bool framebufferResized = false;

        /// GLFW: whenever the window size changed (by OS or user) this callback function executes.
        static void framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
            auto platform = reinterpret_cast<PlatformGl *>(glfwGetWindowUserPointer(window));
            platform->framebufferResized = true;
        }

        void cleanup();

    private:
        void initWindow(uint32_t p_width, uint32_t p_height);
    };
}

#endif //PATHFINDER_GPU_PLATFORM_GL_H
