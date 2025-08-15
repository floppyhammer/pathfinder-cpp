#include <pathfinder/prelude.h>

#include "../common/app.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
#endif

constexpr int32_t WINDOW_WIDTH = 640;
constexpr int32_t WINDOW_HEIGHT = 480;

App* app;
Blit* blit;

void main_loop(void* p_swap_chain) {
    auto* swap_chain = static_cast<Pathfinder::SwapChain*>(p_swap_chain);

    app->update();

    auto encoder = app->device_->create_command_encoder("main encoder");

    auto surface_texture = swap_chain->get_surface_texture();

    // Swap chain render pass.
    {
        encoder->begin_render_pass(swap_chain->get_render_pass(),
                                   surface_texture,
                                   Pathfinder::ColorF(0.2, 0.2, 0.2, 1.0));
        encoder->set_viewport({{0, 0}, swap_chain->size_});

        // Draw canvas to screen.
        blit->draw(encoder);

        encoder->end_render_pass();
    }

    swap_chain->submit(encoder);

    swap_chain->present();
}

int main() {
    // Create the primary window.
    auto window_builder =
        Pathfinder::WindowBuilder::new_impl(Pathfinder::BackendType::Opengl, {WINDOW_WIDTH, WINDOW_HEIGHT});
    auto window = window_builder->get_window(0).lock();

    // Create device and queue.
    auto device = window_builder->request_device();
    auto queue = window_builder->create_queue();

    // Create swap chains for windows.
    auto swap_chain = window->get_swap_chain(device);

    // Create app.
    app = new App(device, queue, window->get_physical_size(), {}, {});

    blit = new Blit(device, queue, swap_chain->get_surface_format());

    {
        const auto dst_texture =
            device->create_texture({window->get_physical_size(), Pathfinder::TextureFormat::Rgba8Unorm}, "dst texture");

        app->canvas_->set_dst_texture(dst_texture);

        blit->set_texture(dst_texture);
    }

    emscripten_set_main_loop_arg(main_loop, swap_chain.get(), 0, 1);

    window_builder->stop_and_destroy_swapchains();

    // Do this after swap chain cleanup.
    delete app;
    delete blit;

    return 0;
}
