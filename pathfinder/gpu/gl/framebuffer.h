#pragma once

#include <memory>

#include "../framebuffer.h"
#include "texture.h"

namespace Pathfinder {

class FramebufferGl : public Framebuffer {
    friend class DeviceGl;
    friend class SwapChainGl;

public:
    ~FramebufferGl() override;

    uint32_t get_gl_handle() const;

private:
    /// Texture framebuffer.
    explicit FramebufferGl(const std::shared_ptr<Texture>& texture);

    /// Swap chain framebuffer.
    FramebufferGl();

    uint32_t gl_framebuffer_{};
};

} // namespace Pathfinder
