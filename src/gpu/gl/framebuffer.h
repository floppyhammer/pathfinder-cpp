#ifndef PATHFINDER_GPU_FRAMEBUFFER_GL_H
#define PATHFINDER_GPU_FRAMEBUFFER_GL_H

#include <memory>

#include "../framebuffer.h"
#include "texture.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class FramebufferGl : public Framebuffer {
public:
    /// Texture framebuffer.
    explicit FramebufferGl(const std::shared_ptr<Texture> &_texture);

    /// Swap chain framebuffer.
    FramebufferGl(Vec2I _size);

    ~FramebufferGl();

    uint32_t get_gl_framebuffer() const;

    unsigned long long get_unique_id() override;

private:
    uint32_t gl_framebuffer;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_FRAMEBUFFER_GL_H
