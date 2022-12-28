#include "../common/app.h"
#include "../src/gpu/platform.h"

const int32_t WINDOW_WIDTH = 1280;
const int32_t WINDOW_HEIGHT = 720;

int main() {
    Vec2I window_size = {WINDOW_WIDTH, WINDOW_HEIGHT};

    auto device_type = Pathfinder::DeviceType::OpenGl4;

    // Create platform.
    auto platform = Pathfinder::Platform::new_impl(device_type, window_size);

    // Create driver via platform.
    auto driver = platform->create_driver();

    // Create swap chain via platform.
    auto swap_chain = platform->create_swap_chain(driver);

    // Create app.
    App app(driver, window_size, {}, {});

    // Set viewport texture to a texture rect.
    auto texture_rect = std::make_shared<TextureRect>(driver, swap_chain->get_render_pass(), window_size.to_f32());
    texture_rect->set_texture(app.canvas->get_dst_texture());

    // Main loop.
    while (!glfwWindowShouldClose(platform->get_glfw_window())) {
        platform->poll_events();

        // Acquire next swap chain image.
        if (!swap_chain->acquire_image()) {
            continue;
        }

        app.update();

        auto cmd_buffer = swap_chain->get_command_buffer();

        auto framebuffer = swap_chain->get_framebuffer();

        // Swap chain render pass.
        {
            cmd_buffer->begin_render_pass(swap_chain->get_render_pass(),
                                          framebuffer,
                                          Pathfinder::ColorF(0.2, 0.2, 0.2, 1.0));

            // Draw canvas to screen.
            texture_rect->draw(driver, cmd_buffer, framebuffer->get_size());

            cmd_buffer->end_render_pass();
        }

        cmd_buffer->submit();

        swap_chain->flush();
    }

    swap_chain->cleanup();

    // Do this after swap chain cleanup.
    app.cleanup();
    texture_rect.reset();

    platform->cleanup();

    return 0;
}
