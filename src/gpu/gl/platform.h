#ifndef PATHFINDER_GPU_PLATFORM_GL_H
#define PATHFINDER_GPU_PLATFORM_GL_H

#include <iostream>
#include <optional>
#include <vector>

#include "../../common/global_macros.h"
#include "../platform.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class PlatformGl : public Platform {
public:
    explicit PlatformGl(Vec2I _window_size);

    std::shared_ptr<Driver> create_driver() override;

    std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver> &driver) override;

    void cleanup() override;

private:
    void init_window();
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_PLATFORM_GL_H
