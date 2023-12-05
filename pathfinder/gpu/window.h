#ifndef PATHFINDER_GPU_WINDOW_H
#define PATHFINDER_GPU_WINDOW_H

#include "device.h"
#include "queue.h"

namespace Pathfinder {

class SwapChain;

class Window {
public:
    explicit Window(Vec2I _size) : size(_size) {}

    virtual ~Window() = default;

    virtual std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Device>& device) = 0;

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

public:
    std::shared_ptr<SwapChain> swapchain;

protected:
    Vec2I size;
    bool just_resized = false;
    bool minimized = false;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_WINDOW_H
