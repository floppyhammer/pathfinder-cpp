#include "../common/app.h"
#include "../src/gpu/window.h"

const int32_t WINDOW_WIDTH = 1280;
const int32_t WINDOW_HEIGHT = 720;

int main() {
    Vec2I window_size = {WINDOW_WIDTH, WINDOW_HEIGHT};

#ifdef PATHFINDER_USE_VULKAN
    auto device_type = DeviceType::Vulkan;
#else
    auto device_type = DeviceType::OpenGl4;
#endif

    // Create a window.
    auto window = Window::new_impl(device_type, window_size);

    // Create a driver via window.
    auto driver = window->create_driver();

    // Create swap chain via window.
    auto swap_chain = window->create_swap_chain(driver);

    // Create app.
    App app(driver, window_size, load_file_as_bytes("../assets/features.svg"), load_file_as_bytes("../assets/sea.png"));

    auto texture_rect = std::make_shared<TextureRect>(driver, swap_chain->get_render_pass());

    {
        auto dst_texture = driver->create_texture({window_size, TextureFormat::Rgba8Unorm, "dst texture"});

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
            auto dst_texture = driver->create_texture({current_window_size, TextureFormat::Rgba8Unorm, "dst texture"});

            app.canvas->set_dst_texture(dst_texture);
            texture_rect->set_texture(dst_texture);
        }

        app.update();

        auto cmd_buffer = swap_chain->get_command_buffer();

        auto framebuffer = swap_chain->get_framebuffer();

        // Swap chain render pass.
        {
            cmd_buffer->begin_render_pass(swap_chain->get_render_pass(), framebuffer, ColorF(0.2, 0.2, 0.2, 1.0));

            // Draw canvas to screen.
            texture_rect->draw(cmd_buffer, framebuffer->get_size());

            cmd_buffer->end_render_pass();
        }

        cmd_buffer->submit();

        swap_chain->flush();
    }

    swap_chain->cleanup();

    // Do this after swap chain cleanup.
    app.cleanup();
    texture_rect.reset();

    window->cleanup();

    return 0;
}
