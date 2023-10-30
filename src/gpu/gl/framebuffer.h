#ifndef PATHFINDER_GPU_FRAMEBUFFER_GL_H
#define PATHFINDER_GPU_FRAMEBUFFER_GL_H

#include <memory>

#include "../framebuffer.h"
#include "texture.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class FramebufferGl : public Framebuffer {
    friend class DeviceGl;
    friend class SwapChainGl;

public:
    ~FramebufferGl() override;

    uint32_t get_gl_framebuffer() const;

    unsigned long long get_unique_id() override;

    void set_label(const std::string& _label) override;

private:
    /// Texture framebuffer.
    explicit FramebufferGl(const std::shared_ptr<Texture>& _texture);

    /// Swap chain framebuffer.
    explicit FramebufferGl(Vec2I _size);

private:
    uint32_t gl_framebuffer{};
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_FRAMEBUFFER_GL_H
