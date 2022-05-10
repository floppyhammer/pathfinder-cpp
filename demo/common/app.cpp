#include "app.h"

App::App(const std::shared_ptr<Pathfinder::Driver> &p_driver,
         const std::shared_ptr<Pathfinder::SwapChain> &swap_chain,
         uint32_t window_width,
         uint32_t window_height,
         std::vector<char> &area_lut_input,
         const std::string &p_svg_input) {
    // Set logger level.
    Pathfinder::Logger::set_level(Pathfinder::Logger::Level::DEBUG);

    driver = p_driver;

    // Set up a canvas.
    canvas = std::make_shared<Pathfinder::Canvas>(driver,
                                                  window_width,
                                                  window_height,
                                                  area_lut_input);
    canvas->load_svg(p_svg_input);

    // Set viewport texture to a texture rect.
    texture_rect = std::make_shared<TextureRect>(driver, swap_chain->get_render_pass(), window_width, window_height);

    // Timers.
    start_time = std::chrono::steady_clock::now();
    last_time = start_time;
    last_time_updated_fps = start_time;
}

void App::loop(const std::shared_ptr<Pathfinder::SwapChain> &swap_chain) {
    // Timing.
    // ----------------------------------------
    auto current_time = std::chrono::steady_clock::now();

    // Time between frames in ms.
    std::chrono::duration<double> duration = current_time - last_time;
    float delta = (float) duration.count() * 1000.f;

    // Time since program started in ms.
    duration = current_time - start_time;
    float elapsed = (float) duration.count() * 1000.f;

    last_time = current_time;

    duration = current_time - last_time_updated_fps;
    if (duration.count() > 1) {
        last_time_updated_fps = current_time;

        // Show frame time.
        std::ostringstream string_stream;
        string_stream << round(delta * 10.f) * 0.1f << "\n";
        std::cout << string_stream.str() << std::endl;
    }
    // ----------------------------------------

    canvas->build_and_render();

    auto cmd_buffer = swap_chain->get_command_buffer();

    auto framebuffer = swap_chain->get_framebuffer();

    // Swap chain render pass.
    {
        cmd_buffer->begin_render_pass(swap_chain->get_render_pass(),
                                      framebuffer,
                                      Pathfinder::ColorF(0.2, 0.2, 0.2, 1.0));

        // Draw canvas to screen.
        texture_rect->set_texture(canvas->get_dest_texture());
        texture_rect->draw(driver, cmd_buffer, framebuffer->get_size());

        cmd_buffer->end_render_pass();
    }

    cmd_buffer->submit(driver);
}

void App::cleanup() {
    texture_rect.reset();
    canvas.reset();
}
