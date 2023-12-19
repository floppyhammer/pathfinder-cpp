#ifndef PATHFINDER_GPU_FRAMEBUFFER_GL_H
#define PATHFINDER_GPU_FRAMEBUFFER_GL_H

#include <memory>

#include "../framebuffer.h"
#include "texture.h"

namespace Pathfinder {

class FramebufferGl : public Framebuffer {
    friend class DeviceGl;
    friend class SwapChainGl;

public:
    ~FramebufferGl() override;

    uint32_t get_gl_framebuffer() const;

    unsigned long long get_unique_id() override;

    void set_label(const std::string& label) override;

private:
    /// Texture framebuffer.
    explicit FramebufferGl(const std::shared_ptr<Texture>& texture);

    /// Swap chain framebuffer.
    explicit FramebufferGl(Vec2I size);

    uint32_t gl_framebuffer_{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_FRAMEBUFFER_GL_H
