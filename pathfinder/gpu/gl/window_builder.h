#ifndef PATHFINDER_WINDOW_BUILDER_GL_H
#define PATHFINDER_WINDOW_BUILDER_GL_H

#include "../window_builder.h"

struct GLFWwindow;

namespace Pathfinder {

class Window;

class WindowBuilderGl : public WindowBuilder {
public:
    explicit WindowBuilderGl(const Vec2I &size);

    ~WindowBuilderGl() override;

    std::shared_ptr<Window> create_window(const Vec2I &size, const std::string &title) override;

    std::shared_ptr<Device> request_device() override;

    std::shared_ptr<Queue> create_queue() override;

private:
#ifndef __ANDROID__
    static GLFWwindow *glfw_window_init(const Vec2I &size,
                                        const std::string &title,
                                        GLFWwindow *shared_window = nullptr);
#endif
};

} // namespace Pathfinder

#endif // PATHFINDER_WINDOW_BUILDER_GL_H
