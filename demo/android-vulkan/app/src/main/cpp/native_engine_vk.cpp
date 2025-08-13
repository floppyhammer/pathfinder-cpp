#include "native_engine_vk.h"

#include <android/log.h>
#include <pathfinder/gpu/vk/window_builder.h>

#include <cassert>
#include <cstring>
#include <vector>

#include "app.h"
#include "vulkan_wrapper.h"

NativeEngineVk::NativeEngineVk(struct android_app *app) : NativeEngine(app) {}

bool NativeEngineVk::init_app() {
    if (!InitVulkan()) {
        Pathfinder::Logger::warn("Vulkan is unavailable, install vulkan and re-start");
        return false;
    }

    auto window_size =
        Pathfinder::Vec2I(ANativeWindow_getWidth(mAppCtx->window), ANativeWindow_getHeight(mAppCtx->window));

    window_builder = std::make_shared<Pathfinder::WindowBuilderVk>(mAppCtx->window, window_size);

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

    return true;
}
