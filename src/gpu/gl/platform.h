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
        explicit PlatformGl(uint32_t window_width, uint32_t window_height);

        std::shared_ptr<Driver> create_driver() override;

        std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver> &driver) override;

        void cleanup() override;

    private:
        void init_window();
    };
}

#endif

#endif //PATHFINDER_GPU_PLATFORM_GL_H
