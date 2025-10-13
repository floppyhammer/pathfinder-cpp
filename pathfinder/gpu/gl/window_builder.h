#ifndef PATHFINDER_WINDOW_BUILDER_GL_H
#define PATHFINDER_WINDOW_BUILDER_GL_H

#include "../window_builder.h"

struct GLFWwindow;

namespace Pathfinder {

class Window;

class WindowBuilderGl : public WindowBuilder {
public:
#ifdef __ANDROID__
    WindowBuilderGl(ANativeWindow *native_window, const Vec2I &window_size);
#else
    explicit WindowBuilderGl(const Vec2I &size);
#endif

    ~WindowBuilderGl() override;

    uint8_t create_window(const Vec2I &size, const std::string &title) override;

    std::shared_ptr<Device> request_device() override;

    std::shared_ptr<Queue> create_queue() override;

private:
#ifdef __ANDROID__
    bool init_display();

    bool init_surface();

    bool init_context();

    EGLDisplay egl_display_;
    EGLSurface egl_surface_;
    EGLContext egl_context_;
    EGLConfig egl_config_;
#endif
};

} // namespace Pathfinder

#endif // PATHFINDER_WINDOW_BUILDER_GL_H
