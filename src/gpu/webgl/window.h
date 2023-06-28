#ifndef PATHFINDER_GPU_WINDOW_WEBGL_H
#define PATHFINDER_GPU_WINDOW_WEBGL_H

#include <iostream>
#include <vector>

#include "../../common/global_macros.h"
#include "../window.h"

#ifdef __EMSCRIPTEN__

namespace Pathfinder {

class WindowWebGl : public Window {
public:
    explicit WindowWebGl(Vec2I _size);

    std::shared_ptr<Device> create_device() override;

    std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Device> &device) override;

    void cleanup() override;

private:
    void init();
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_WINDOW_WEBGL_H
