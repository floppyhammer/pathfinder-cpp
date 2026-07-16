#include "swap_chain.h"

#include "command_encoder.h"
#include "device.h"
#include "queue.h"
#include "render_pass.h"

namespace Pathfinder {

SwapChainMtl::SwapChainMtl(
    const Vec2I& size, const std::shared_ptr<DeviceMtl>& device, CAMetalLayer* layer, PresentMode present_mode)
    : SwapChain(size, present_mode), device_(device), layer_(layer) {
    layer_.device = device_->get_handle();

    // Enable/disable V-Sync based on present mode.
    // In Metal, displaySyncEnabled = YES corresponds to FIFO (V-Sync on).
    // displaySyncEnabled = NO corresponds to Immediate (V-Sync off).
    layer_.displaySyncEnabled = (present_mode != PresentMode::Immediate);

    render_pass_ = device_->create_swap_chain_render_pass(get_surface_format(), AttachmentLoadOp::Clear);
}

std::shared_ptr<RenderPass> SwapChainMtl::get_render_pass() {
    return render_pass_;
}

std::shared_ptr<Texture> SwapChainMtl::get_surface_texture() {
    if (current_drawable_) {
        return device_->wrap_texture(current_drawable_.texture, false);
    }
    return nullptr;
}

TextureFormat SwapChainMtl::get_surface_format() const {
    return from_mtl_pixel_format(layer_.pixelFormat);
}

bool SwapChainMtl::acquire_image() {
    current_drawable_ = [layer_ nextDrawable];
    return current_drawable_ != nil;
}

void SwapChainMtl::submit(const std::shared_ptr<CommandEncoder>& encoder) {
    if (current_drawable_) {
        auto encoder_mtl = (CommandEncoderMtl*)encoder.get();
        // Since prepare() might create the command buffer, we should call it or ensure it's called.
        // However, prepare() has an assert(mtl_cmd_buffer_ == nil), so it can only be called once.
        // QueueMtl::submit() also calls prepare().
        // For now, let's let QueueMtl::submit() do the heavy lifting.
    }

    device_->get_queue()->submit(encoder, nullptr);
}

void SwapChainMtl::present() {
    if (current_drawable_) {
        [current_drawable_ present];
        current_drawable_ = nil;
    }
}

void SwapChainMtl::destroy() {
    current_drawable_ = nil;
}

} // namespace Pathfinder
