#include "VulkanMain.hpp"
#include "vulkan_wrapper.h"

#include "app.h"
#include <pathfinder/gpu/vk/window_builder.h>

#include <android/log.h>

#include <cassert>
#include <cstring>
#include <vector>

// Android Native App pointer.
android_app *androidAppCtx = nullptr;

Pathfinder::WindowBuilderVk *window_builder{};

std::shared_ptr<App> pf_app;
std::shared_ptr<TextureRect> pf_text_rect;

std::shared_ptr<Pathfinder::Window> pf_window;
std::shared_ptr<Pathfinder::Device> pf_device;
std::shared_ptr<Pathfinder::Queue> pf_queue;
std::shared_ptr<Pathfinder::SwapChain> pf_swapchain;

// InitVulkan:
//   Initialize Vulkan Context when android application window is created
//   upon return, vulkan is ready to draw frames
bool InitVulkan(android_app *app) {
    androidAppCtx = app;

    if (!InitVulkan()) {
        Pathfinder::Logger::warn("Vulkan is unavailable, install vulkan and re-start");
        return false;
    }

    auto window_size = Pathfinder::Vec2I(
            ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window));

    window_builder = new Pathfinder::WindowBuilderVk(app->window, window_size);

    pf_window = window_builder->get_main_window();

    // Create device and queue.
    pf_device = window_builder->request_device();
    pf_queue = window_builder->create_queue();

    // Create swap chains for windows.
    pf_swapchain = pf_window->create_swap_chain(pf_device);

    auto svg_input = Pathfinder::load_asset(androidAppCtx->activity->assetManager, "features.svg");
    auto img_input = Pathfinder::load_asset(androidAppCtx->activity->assetManager, "sea.png");

    // Create app.
    pf_app = std::make_shared<App>(pf_device,
                                   pf_queue,
                                   window_size,
                                   svg_input,
                                   img_input);

    pf_text_rect = std::make_shared<TextureRect>(pf_device, pf_queue,
                                                 pf_swapchain->get_render_pass());

    {
        auto dst_texture = pf_device->create_texture(
                {window_size, Pathfinder::TextureFormat::Rgba8Unorm}, "dst texture");

        pf_app->canvas->set_dst_texture(dst_texture);

        pf_text_rect->set_texture(dst_texture);
    }

    return true;
}

// Native app poll to see if we are ready to draw.
bool IsVulkanReady(void) { return window_builder && window_builder->get_device(); }

void DeleteVulkan(void) {
    delete window_builder;
    window_builder = nullptr;
}

bool VulkanDrawFrame(void) {
    // Acquire next swap chain image.
    if (!pf_swapchain->acquire_image()) {
        return false;
    }

    auto current_window_size = pf_window->get_size();

    if (current_window_size != pf_app->canvas->get_dst_texture()->get_size() &&
        current_window_size.area() != 0) {
        auto dst_texture =
                pf_device->create_texture(
                        {current_window_size, Pathfinder::TextureFormat::Rgba8Unorm},
                        "dst texture");

        pf_app->canvas->set_dst_texture(dst_texture);
        pf_text_rect->set_texture(dst_texture);

        pf_app->canvas->set_size(current_window_size);
    }

    pf_app->update();

    auto encoder = pf_device->create_command_encoder("Main encoder");

    auto framebuffer = pf_swapchain->get_framebuffer();

    // Swap chain render pass.
    {
        encoder->begin_render_pass(pf_swapchain->get_render_pass(), framebuffer,
                                   Pathfinder::ColorF(0.2, 0.2, 0.2, 1.0));

        // Draw canvas to screen.
        pf_text_rect->draw(encoder, framebuffer->get_size());

        encoder->end_render_pass();
    }

    pf_queue->submit(encoder, pf_swapchain);

    pf_swapchain->present();

    return true;
}
