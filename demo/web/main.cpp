#include "../common/app.h"
#include "../src/gpu/window.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
#endif

using namespace Pathfinder;

const int32_t WINDOW_WIDTH = 640;
const int32_t WINDOW_HEIGHT = 480;

App* app;
TextureRect* texture_rect;

void render(void* _swap_chain) {
    app->update();

    auto* swap_chain = static_cast<SwapChain*>(_swap_chain);

    auto encoder = app->device->create_command_encoder("Main encoder");

    auto framebuffer = swap_chain->get_framebuffer();

    // Swap chain render pass.
    {
        encoder->begin_render_pass(swap_chain->get_render_pass(), framebuffer, Pathfinder::ColorF(0.2, 0.2, 0.2, 1.0));

        // Draw canvas to screen.
        texture_rect->draw(encoder, framebuffer->get_size());

        encoder->end_render_pass();
    }

    app->queue->submit_and_wait(encoder);

    swap_chain->present();
}

int main() {
    Vec2I window_size = {WINDOW_WIDTH, WINDOW_HEIGHT};

    auto window = Window::new_impl(window_size);

    auto device = window->request_device();

    auto queue = window->create_queue();

    auto swap_chain = window->create_swap_chain(device);

    // Create app.
    app = new App(device, queue, {}, {});

    auto dst_texture = device->create_texture({window_size, TextureFormat::Rgba8Unorm}, "dst texture");

    app->canvas->set_dst_texture(dst_texture);

    // Set viewport texture to a texture rect.
    texture_rect = new TextureRect(device, queue, swap_chain->get_render_pass());
    texture_rect->set_texture(dst_texture);

    emscripten_set_main_loop_arg(render, swap_chain.get(), 0, 1);

    swap_chain->cleanup();

    // Do this after swap chain cleanup.
    delete app;
    delete texture_rect;

    window->cleanup();

    return 0;
}
