#ifndef PATHFINDER_GPU_WINDOW_GL_H
#define PATHFINDER_GPU_WINDOW_GL_H

#include "../window.h"

struct GLFWwindow;

namespace Pathfinder {

class WindowGl : public Window {
    friend class WindowBuilderGl;

public:
#ifndef __ANDROID__
    WindowGl(const Vec2I &size, GLFWwindow *window_handle);
#else
    explicit WindowGl(const Vec2I &size);
#endif

    std::shared_ptr<SwapChain> get_swap_chain(const std::shared_ptr<Device> &device) override;

private:
    void destroy() override;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_WINDOW_GL_H
