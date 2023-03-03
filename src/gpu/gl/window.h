#ifndef PATHFINDER_GPU_WINDOW_GL_H
#define PATHFINDER_GPU_WINDOW_GL_H

#include <iostream>
#include <vector>

#include "../../common/global_macros.h"
#include "../window.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class WindowGl : public Window {
public:
    explicit WindowGl(Vec2I _window_size);

    std::shared_ptr<Driver> create_driver() override;

    std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver> &driver) override;

    void cleanup() override;

private:
    void init();
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_WINDOW_GL_H
