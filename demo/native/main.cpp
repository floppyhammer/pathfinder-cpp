#include <pathfinder/prelude.h>

#include "../common/app.h"

const int32_t WINDOW_WIDTH = 1280;
const int32_t WINDOW_HEIGHT = 720;

int main() {
    Pathfinder::Vec2I window_size = {WINDOW_WIDTH, WINDOW_HEIGHT};

    // Create the primary window.
    auto window_builder = Pathfinder::WindowBuilder::new_impl(window_size);
    auto window = window_builder->get_primary_window();

    // Create device and queue.
    auto device = window_builder->request_device();
    auto queue = window_builder->create_queue();

    // Create swap chains for windows.
    auto swap_chain = window->get_swap_chain(device);

    // Create app.
    App app(device,
            queue,
            window_size,
            Pathfinder::load_file_as_bytes("../assets/features.svg"),
            Pathfinder::load_file_as_bytes("../assets/sea.png"));

    auto texture_rect = std::make_shared<TextureRect>(device, queue, swap_chain->get_render_pass());

    {
        auto dst_texture = device->create_texture({window_size, Pathfinder::TextureFormat::Rgba8Unorm}, "dst texture");

        app.canvas->set_dst_texture(dst_texture);

        texture_rect->set_texture(dst_texture);
    }

    // Main loop.
    while (!window->should_close()) {
        // Currently, multiple window does not work properly for the GL backend.
        for (int i = 0; i < 1; i++) {
            window->poll_events();

            // Acquire next swap chain image.
            if (!swap_chain->acquire_image()) {
                continue;
            }

            auto current_window_size = window->get_size();

            if (current_window_size != app.canvas->get_dst_texture()->get_size() && current_window_size.area() != 0) {
                auto dst_texture =
                    device->create_texture({current_window_size, Pathfinder::TextureFormat::Rgba8Unorm}, "dst texture");

                app.canvas->set_dst_texture(dst_texture);
                texture_rect->set_texture(dst_texture);

                app.canvas->set_size(current_window_size);
            }

            app.update();

            auto encoder = device->create_command_encoder("Main encoder");

            auto framebuffer = swap_chain->get_framebuffer();

            // Swap chain render pass.
            {
                encoder->begin_render_pass(swap_chain->get_render_pass(),
                                           framebuffer,
                                           Pathfinder::ColorF(0.2, 0.2, 0.2, 1.0));

                // Draw canvas to screen.
                texture_rect->draw(encoder, framebuffer->get_size());

                encoder->end_render_pass();
            }

            queue->submit(encoder, swap_chain);

            swap_chain->present();
        }
    }

    window_builder->preapre_destruction();

    // Do this after swap chain cleanup.
    app.destroy();
    texture_rect.reset();

    return 0;
}
