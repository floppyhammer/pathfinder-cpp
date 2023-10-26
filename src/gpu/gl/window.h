#ifndef PATHFINDER_GPU_WINDOW_GL_H
#define PATHFINDER_GPU_WINDOW_GL_H

#include <iostream>
#include <vector>

#include "../../common/global_macros.h"
#include "../window.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class WindowGl : public Window {
    friend class WindowBuilderGl;

public:
    explicit WindowGl(const Vec2I &_size, GLFWwindow *window_handle);

    ~WindowGl() override = default;

    std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Device> &device) override;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_WINDOW_GL_H
