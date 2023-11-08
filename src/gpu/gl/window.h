#ifndef PATHFINDER_GPU_WINDOW_GL_H
#define PATHFINDER_GPU_WINDOW_GL_H

#include <iostream>
#include <vector>

#include "../../common/global_macros.h"
#include "../window.h"

struct GLFWwindow;

namespace Pathfinder {

class WindowGl : public Window {
    friend class WindowBuilderGl;

public:
#ifndef __ANDROID__
    explicit WindowGl(const Vec2I &_size, GLFWwindow *window_handle);
#else
    explicit WindowGl(const Vec2I &_size);
#endif

    ~WindowGl() override;

    std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Device> &device) override;

#ifndef __ANDROID__
public:
    GLFWwindow *get_glfw_window() const;

    /// GLFW: whenever the window size changed (by OS or user) this callback function executes.
    static void framebuffer_resize_callback(GLFWwindow *glfw_window, int width, int height);

    /// Process input events: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
    void poll_events() override;

    bool should_close() override;
#endif

private:
    void destroy();

#ifndef __ANDROID__
    GLFWwindow *glfw_window{};
#endif
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_WINDOW_GL_H
