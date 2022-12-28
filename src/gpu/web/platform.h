#ifndef PATHFINDER_GPU_PLATFORM_WEBGL_H
#define PATHFINDER_GPU_PLATFORM_WEBGL_H

#include <iostream>
#include <optional>
#include <vector>

#include "../../common/global_macros.h"
#include "../platform.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class PlatformWebGl : public Platform {
public:
    explicit PlatformWebGl(Vec2I _window_size);

    std::shared_ptr<Driver> create_driver() override;

    std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver> &driver) override;

    void cleanup() override;

private:
    void init_window();
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_PLATFORM_WEBGL_H
