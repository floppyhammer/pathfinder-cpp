#ifndef PATHFINDER_GPU_WINDOW_H
#define PATHFINDER_GPU_WINDOW_H

#include "device.h"
#include "queue.h"
#include "swap_chain.h"

namespace Pathfinder {

class SwapChain;

class Window {
public:
    explicit Window(const Vec2I& size) : size_(size) {}

    virtual ~Window() = default;

    virtual void destroy() = 0;

    virtual std::shared_ptr<SwapChain> get_swap_chain(const std::shared_ptr<Device>& device) = 0;

    Vec2I get_size() const;

    bool get_resize_flag() const;

    bool is_minimized() const;

    virtual void* get_raw_handle() const;

#ifndef __ANDROID__
    /// Process input events: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
    virtual void poll_events() {}

    virtual bool should_close() {
        return false;
    }
#endif

    std::shared_ptr<SwapChain> swapchain_;

protected:
    Vec2I size_;
    bool just_resized_ = false;
    bool minimized_ = false;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_WINDOW_H
