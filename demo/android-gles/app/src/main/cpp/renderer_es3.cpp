#include "renderer_es3.h"

#include <iostream>
#include <chrono>

RendererES *createES3Renderer(int width, int height, AAssetManager *_asset_manager) {
    auto *renderer = new RendererES3(_asset_manager);
    renderer->init(width, height);

    return renderer;
}

RendererES3::RendererES3(AAssetManager *_asset_manager)
        : RendererES(_asset_manager), egl_context(eglGetCurrentContext()) {}

void RendererES3::init(int width, int height) {
    ALOGV("Using OpenGL ES 3.0 renderer");

    window_size = {width, height};

    pf_window_builder = Pathfinder::WindowBuilder::new_impl(window_size);
    pf_window = pf_window_builder->get_primary_window();
    pf_device = pf_window_builder->request_device();
    pf_queue = pf_window_builder->create_queue();
    pf_swapchain = pf_window->get_swap_chain(pf_device);

    auto svg_input = Pathfinder::load_asset(asset_manager, "features.svg");
    auto img_input = Pathfinder::load_asset(asset_manager, "sea.png");

    pf_app = std::make_shared<App>(pf_device, pf_queue, window_size, svg_input, img_input);

    auto dst_texture = pf_device->create_texture(
            {window_size, Pathfinder::TextureFormat::Rgba8Unorm}, "dst texture");

    pf_app->canvas_->set_dst_texture(dst_texture);

    pf_texture_rect = std::make_shared<TextureRect>(pf_device, pf_queue, nullptr);
    pf_texture_rect->set_texture(dst_texture);
}

RendererES3::~RendererES3() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
    if (eglGetCurrentContext() != egl_context) {
        return;
    }
}

void RendererES3::render() {
    // Acquire next swap chain image.
    if (!pf_swapchain->acquire_image()) {
        return;
    }

    pf_app->update();

    auto encoder = pf_device->create_command_encoder("Main encoder");

    auto framebuffer = pf_swapchain->get_framebuffer();

    // Swap chain render pass.
    {
        encoder->begin_render_pass(pf_swapchain->get_render_pass(), framebuffer,
                                   Pathfinder::ColorF(0.2, 0.2, 0.2, 1.0));

        // Draw canvas to screen.
        pf_texture_rect->draw(encoder, framebuffer->get_size());

        encoder->end_render_pass();
    }

    pf_queue->submit(encoder, pf_swapchain);

    pf_swapchain->present();
}
