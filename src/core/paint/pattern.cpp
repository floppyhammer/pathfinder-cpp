#include "pattern.h"

namespace Pathfinder {

RenderTarget::RenderTarget(const std::shared_ptr<Driver>& driver, const Vec2I& _size, const std::string& label) {
    size = _size;

    render_pass =
        driver->create_render_pass(TextureFormat::Rgba8Unorm, AttachmentLoadOp::Clear, false, label + "render pass");

    auto target_texture = driver->create_texture(size, TextureFormat::Rgba8Unorm, label + " texture");

    // Create a new framebuffer.
    framebuffer = driver->create_framebuffer(render_pass, target_texture, label + "framebuffer");
}

bool Pattern::repeat_x() const {
    return (flags.value & PatternFlags::REPEAT_X) != 0x0;
}

void Pattern::set_repeat_x(bool repeat_x) {
    if (repeat_x) {
        flags.value |= PatternFlags::REPEAT_X;
    } else {
        flags.value &= ~PatternFlags::REPEAT_X;
    }
}

bool Pattern::repeat_y() const {
    return (flags.value & PatternFlags::REPEAT_Y) != 0x0;
}

void Pattern::set_repeat_y(bool repeat_y) {
    if (repeat_y) {
        flags.value |= PatternFlags::REPEAT_Y;
    } else {
        flags.value &= ~PatternFlags::REPEAT_Y;
    }
}

bool Pattern::smoothing_enabled() const {
    return (flags.value & PatternFlags::NO_SMOOTHING) == 0x0;
}

void Pattern::set_smoothing_enabled(bool enable) {
    if (!enable) {
        flags.value |= PatternFlags::NO_SMOOTHING;
    } else {
        flags.value &= ~PatternFlags::NO_SMOOTHING;
    }
}

} // namespace Pathfinder
