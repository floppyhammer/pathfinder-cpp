#ifndef PATHFINDER_GPU_PLATFORM_GL_H
#define PATHFINDER_GPU_PLATFORM_GL_H

#include "../platform.h"
#include "../../common/global_macros.h"

#include <vector>
#include <iostream>
#include <optional>

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class PlatformGl : public Platform {
    public:
        static std::shared_ptr<Platform> create(uint32_t p_width, uint32_t p_height) {
            auto platform_gl = std::make_shared<PlatformGl>();
            platform_gl->init(p_width, p_height);
            return platform_gl;
        }

        void init(uint32_t p_width, uint32_t p_height);

        std::shared_ptr<Driver> create_driver() override;

        std::shared_ptr<SwapChain> create_swap_chain(uint32_t p_width, uint32_t p_height) override;

        void poll_events() const override;

        bool framebufferResized = false;

        /// GLFW: whenever the window size changed (by OS or user) this callback function executes.
        static void framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
            auto platform = reinterpret_cast<PlatformGl *>(glfwGetWindowUserPointer(window));
            platform->framebufferResized = true;
        }

        void cleanup() override;

    private:
        void initWindow(uint32_t p_width, uint32_t p_height);
    };
}

#endif

#endif //PATHFINDER_GPU_PLATFORM_GL_H
