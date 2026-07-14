#pragma once

#include "../framebuffer.h"

namespace Pathfinder {

class FramebufferMtl final : public Framebuffer {
public:
    FramebufferMtl(const std::shared_ptr<Texture> &texture, const std::string &label) : Framebuffer(texture) {}

    ~FramebufferMtl() override = default;
};

} // namespace Pathfinder
