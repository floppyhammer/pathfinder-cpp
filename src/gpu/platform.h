#ifndef PATHFINDER_GPU_PLATFORM_H
#define PATHFINDER_GPU_PLATFORM_H

#include "gl/driver.h"
#include "vk/driver.h"

namespace Pathfinder {
    class Platform {
    public:
        inline GLFWwindow *get_glfw_window() const {
            return window;
        }

        std::shared_ptr<Driver> driver;

    protected:
        GLFWwindow *window;
    };
}

#endif //PATHFINDER_GPU_PLATFORM_H
