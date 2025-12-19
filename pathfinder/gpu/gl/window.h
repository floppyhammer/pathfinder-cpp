#ifndef PATHFINDER_GPU_WINDOW_GL_H
#define PATHFINDER_GPU_WINDOW_GL_H

#ifdef __ANDROID__
    #include <EGL/egl.h>
#endif

#include "../window.h"

namespace Pathfinder {

class WindowGl : public Window {
    friend class WindowBuilderGl;

public:
#ifndef __ANDROID__
    WindowGl(const Vec2I &size, void *window_handle);
#else
    WindowGl(const Vec2I &size, EGLDisplay egl_display, EGLSurface egl_surface, EGLContext egl_context);
#endif

    std::shared_ptr<SwapChain> get_swap_chain(const std::shared_ptr<Device> &device) override;

private:
    void destroy() override;

#ifdef __ANDROID__
    EGLDisplay egl_display_;
    EGLSurface egl_surface_;
    EGLContext egl_context_;
#endif
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_WINDOW_GL_H
