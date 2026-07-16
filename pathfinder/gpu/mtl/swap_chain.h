#pragma once

#import <QuartzCore/CAMetalLayer.h>

#include "../swap_chain.h"
#include "device.h"
#include "texture.h"

namespace Pathfinder {

class SwapChainMtl : public SwapChain {
public:
    SwapChainMtl(
        const Vec2I& size, const std::shared_ptr<DeviceMtl>& device, CAMetalLayer* layer, PresentMode present_mode);

    std::shared_ptr<RenderPass> get_render_pass() override;

    std::shared_ptr<Texture> get_surface_texture() override;

    TextureFormat get_surface_format() const override;

    bool acquire_image() override;

    void submit(const std::shared_ptr<CommandEncoder>& encoder) override;

    void present() override;

    void destroy() override;

private:
    std::shared_ptr<DeviceMtl> device_;
    CAMetalLayer* layer_{};
    id<CAMetalDrawable> current_drawable_ = nil;
    std::shared_ptr<TextureMtl> surface_texture_;
    std::shared_ptr<RenderPass> render_pass_;
};

} // namespace Pathfinder
