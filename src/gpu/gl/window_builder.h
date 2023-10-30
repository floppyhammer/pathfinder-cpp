#ifndef PATHFINDER_WINDOW_BUILDER_GL_H
#define PATHFINDER_WINDOW_BUILDER_GL_H

#include <memory>
#include <vector>

#include "../window_builder.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class Window;

class WindowBuilderGl : public WindowBuilder {
public:
    explicit WindowBuilderGl(const Vec2I& size);

    ~WindowBuilderGl() override;

    std::shared_ptr<Window> create_window(const Vec2I& _size, const std::string& title) override;

    void destroy_window(const std::shared_ptr<Window>& window) override;

    std::shared_ptr<Device> request_device() override;

    std::shared_ptr<Queue> create_queue() override;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_WINDOW_BUILDER_GL_H
