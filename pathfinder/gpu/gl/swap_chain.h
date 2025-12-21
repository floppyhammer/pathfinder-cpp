#ifndef PATHFINDER_GPU_SWAP_CHAIN_GL_H
#define PATHFINDER_GPU_SWAP_CHAIN_GL_H

#ifdef __ANDROID__
    #include <EGL/egl.h>
#endif

#include "../swap_chain.h"
#include "base.h"
#include "command_encoder.h"
#include "framebuffer.h"

namespace Pathfinder {

class SwapChainGl : public SwapChain {
    friend class DeviceGl;
    friend class CommandEncoderGl;

public:
#ifndef __ANDROID__
    SwapChainGl(Vec2I size, void *window_handle);
#else
    SwapChainGl(Vec2I size, EGLDisplay egl_display, EGLSurface egl_surface, EGLContext egl_context);
#endif

    std::shared_ptr<RenderPass> get_render_pass() override {
        return render_pass_;
    }

    std::shared_ptr<Texture> get_surface_texture() override {
        return nullptr;
    }

    TextureFormat get_surface_format() const override {
        return TextureFormat::Rgba8Unorm;
    }

    bool acquire_image() override;

    void submit(const std::shared_ptr<CommandEncoder> &encoder) override;

    void present() override;

    void destroy() override {}

private:
#ifndef __ANDROID__
    void *glfw_window_;
#else
    EGLDisplay egl_display_;
    EGLSurface egl_surface_;
    EGLContext egl_context_;
#endif

    std::shared_ptr<RenderPass> render_pass_;
    std::shared_ptr<CommandEncoder> command_encoder_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_SWAP_CHAIN_GL_H
