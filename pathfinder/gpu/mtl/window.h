#pragma once

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

#include "../window.h"

namespace Pathfinder {

class WindowMtl : public Window {
public:
    WindowMtl(const Vec2I& size, void* glfw_window);

    ~WindowMtl() override;

    void destroy() override;

    std::shared_ptr<SwapChain> get_swap_chain(const std::shared_ptr<Device>& device) override;

    CAMetalLayer* get_metal_layer() const {
        return metal_layer_;
    }

private:
    CAMetalLayer* metal_layer_ = nil;
};

} // namespace Pathfinder
