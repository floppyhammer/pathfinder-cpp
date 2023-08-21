#include "../common/app.h"
#include "../src/gpu/window.h"

const int32_t WINDOW_WIDTH = 1280;
const int32_t WINDOW_HEIGHT = 720;

int main() {
    Vec2I window_size = {WINDOW_WIDTH, WINDOW_HEIGHT};

    // Create a window.
    auto window = Window::new_impl(window_size);

    // Create a device via window.
    auto device = window->request_device();

    auto queue = window->create_queue();

    // Create swap chain via window.
    auto swap_chain = window->create_swap_chain(device);

    // Create app.
    App app(device,
            queue,
            window_size,
            load_file_as_bytes("../assets/features.svg"),
            load_file_as_bytes("../assets/sea.png"));

    auto texture_rect = std::make_shared<TextureRect>(device, queue, swap_chain->get_render_pass());

    {
        auto dst_texture = device->create_texture({window_size, TextureFormat::Rgba8Unorm}, "dst texture");

        app.canvas->set_dst_texture(dst_texture);

        texture_rect->set_texture(dst_texture);
    }

    // Main loop.
    while (!window->should_close()) {
        window->poll_events();

        // Acquire next swap chain image.
        if (!swap_chain->acquire_image()) {
            continue;
        }

        auto current_window_size = window->get_size();

        if (current_window_size != app.canvas->get_size() && current_window_size.area() != 0) {
            app.canvas->set_size(current_window_size);

            auto dst_texture = device->create_texture({current_window_size, TextureFormat::Rgba8Unorm}, "dst texture");
            app.canvas->set_dst_texture(dst_texture);
            texture_rect->set_texture(dst_texture);
        }

        app.update();

        auto encoder = device->create_command_encoder("Main encoder");

        auto framebuffer = swap_chain->get_framebuffer();

        // Swap chain render pass.
        {
            encoder->begin_render_pass(swap_chain->get_render_pass(), framebuffer, ColorF(0.2, 0.2, 0.2, 1.0));

            // Draw canvas to screen.
            texture_rect->draw(encoder, framebuffer->get_size());

            encoder->end_render_pass();
        }

        queue->submit(encoder, swap_chain);

        swap_chain->present();
    }

    swap_chain->cleanup();

    // Do this after swap chain cleanup.
    app.cleanup();
    texture_rect.reset();

    window->cleanup();

    return 0;
}
