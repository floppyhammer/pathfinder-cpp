#include "native_engine.h"

// clang-format off
#include "vulkan_wrapper.h"
#include "pathfinder/gpu/gl/window_builder.h"
#include "pathfinder/gpu/vk/window_builder.h"
// clang-format on

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

    pf_swapchain->submit(encoder);

    pf_swapchain->present();
}

bool NativeEngine::is_ready() const {
    return window_builder != nullptr;
}

void NativeEngine::init_app_common(Pathfinder::Vec2I window_size) {
    pf_window = window_builder->get_window(0).lock();

    // Create device and queue.
    pf_device = window_builder->request_device();
    pf_queue = window_builder->create_queue();

    // Create swap chains for windows.
    pf_swapchain = pf_window->get_swap_chain(pf_device);

    auto svg_input = Pathfinder::load_asset(mAppCtx->activity->assetManager, "features.svg");
    auto img_input = Pathfinder::load_asset(mAppCtx->activity->assetManager, "sea.png");

    // Create app.
    pf_app = std::make_shared<App>(pf_device, pf_queue, window_size, svg_input, img_input);

    pf_blit = std::make_shared<Blit>(pf_device, pf_queue, pf_swapchain->get_surface_format());

    auto dst_texture = pf_device->create_texture({window_size, Pathfinder::TextureFormat::Rgba8Unorm}, "dst texture");

    pf_app->canvas_->set_dst_texture(dst_texture);

    pf_blit->set_texture(dst_texture);
}

bool NativeEngine::init_app(bool use_vulkan) {
    auto window_size =
        Pathfinder::Vec2I(ANativeWindow_getWidth(mAppCtx->window), ANativeWindow_getHeight(mAppCtx->window));

    if (!use_vulkan) {
        window_builder = std::make_shared<Pathfinder::WindowBuilderGl>(mAppCtx->window, window_size);
    } else {
        if (!InitVulkan()) {
            Pathfinder::Logger::warn("Vulkan is unavailable, install vulkan and re-start");
            return false;
        }

        window_builder = std::make_shared<Pathfinder::WindowBuilderVk>(mAppCtx->window, window_size);
    }

    init_app_common(window_size);

    return true;
}
