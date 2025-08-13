#include "native_engine.h"

void NativeEngine::draw_frame() {
    // Acquire next swap chain image.
    if (!pf_swapchain->acquire_image()) {
        return;
    }

    auto current_window_size = pf_window->get_logical_size();

    if (current_window_size != pf_app->canvas_->get_dst_texture()->get_size() && current_window_size.area() != 0) {
        auto dst_texture =
            pf_device->create_texture({current_window_size, Pathfinder::TextureFormat::Rgba8Unorm}, "dst texture");

        pf_app->canvas_->set_dst_texture(dst_texture);
        pf_blit->set_texture(dst_texture);

        pf_app->canvas_->set_size(current_window_size);
    }

    pf_app->update();

    auto encoder = pf_device->create_command_encoder("main encoder");

    auto surface = pf_swapchain->get_surface_texture();

    // Swap chain render pass.
    {
        encoder->begin_render_pass(pf_swapchain->get_render_pass(), surface, Pathfinder::ColorF(0.2, 0.2, 0.2, 1.0));

        encoder->set_viewport({{0, 0}, pf_swapchain->size_});

        // Draw canvas to screen.
        pf_blit->draw(encoder);

        encoder->end_render_pass();
    }

    pf_queue->submit(encoder, pf_swapchain);

    pf_swapchain->present();
}

bool NativeEngine::is_ready() const {
    return window_builder != nullptr;
}
