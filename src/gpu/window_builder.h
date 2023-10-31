#ifndef PATHFINDER_WINDOW_BUILDER_H
#define PATHFINDER_WINDOW_BUILDER_H

#include "device.h"
#include "queue.h"

namespace Pathfinder {

class Window;

/// Window management.
class WindowBuilder {
public:
    static std::shared_ptr<WindowBuilder> new_impl(const Vec2I &size);

    virtual ~WindowBuilder() = default;

    /// Create a new sub-window.
    virtual std::shared_ptr<Window> create_window(const Vec2I &size, const std::string &title) = 0;

    std::shared_ptr<Window> get_main_window() const;

#ifndef __ANDROID__
    static GLFWwindow *common_glfw_window_init(const Vec2I &size,
                                               const std::string &title,
                                               GLFWwindow *shared_window = nullptr);
#endif

    virtual std::shared_ptr<Device> request_device() = 0;

    virtual std::shared_ptr<Queue> create_queue() = 0;

protected:
    std::shared_ptr<Window> main_window;
    std::vector<std::weak_ptr<Window>> sub_windows;
};

} // namespace Pathfinder

#endif // PATHFINDER_WINDOW_BUILDER_H