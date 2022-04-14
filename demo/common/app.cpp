//
// Created by floppyhammer on 2/24/2022.
//

#include "app.h"

#include "../../src/common/global_macros.h"
#include "../../src/common/logger.h"
#include "../../src/rendering/device.h"

App::App(int window_width,
         int window_height,
         std::vector<char> &area_lut_input,
         std::vector<char> &font_input,
         const std::string &p_svg_input) {
    // Set logger level.
    Pathfinder::Logger::set_level(Pathfinder::Logger::Level::DEBUG);

    // Set up a canvas.
    canvas = std::make_shared<Pathfinder::Canvas>(window_width,
                                                  window_height,
                                                  reinterpret_cast<std::vector<unsigned char> &>(area_lut_input));
    canvas->load_svg(p_svg_input);

    // Set up a text label.
    label = std::make_shared<Pathfinder::Label>(256,
                                                64,
                                                reinterpret_cast<std::vector<unsigned char> &>(area_lut_input));
    label->set_style(64, Pathfinder::ColorU::white(), 0, Pathfinder::ColorU::red());
    label->set_font(std::make_shared<Pathfinder::Font>(font_input));
    label->set_horizontal_alignment(Pathfinder::Alignment::Center);

    // Create a screen viewport.
    screen_framebuffer = std::make_shared<Pathfinder::Framebuffer>(window_width, window_height);

    // Set viewport texture to a texture rect.
    texture_rect0 = std::make_shared<Pathfinder::TextureRect>(window_width, window_height);
    texture_rect1 = std::make_shared<Pathfinder::TextureRect>(label->get_rect_size().x,
                                                              label->get_rect_size().y);

    // Timers.
    start_time = std::chrono::steady_clock::now();
    last_time = start_time;
    last_time_updated_fps = start_time;
}

void App::loop() {
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

        // Set frame time.
        std::ostringstream string_stream;
        string_stream << round(delta * 10.f) * 0.1f << "\n";
        label->set_text(string_stream.str());
    }
    // ----------------------------------------

    // Build and draw canvas.
    canvas->update();
    canvas->draw();

    // Build and draw label.
    label->draw();

    auto cmd_buffer = Pathfinder::Device::create_command_buffer();

    cmd_buffer->begin_render_pass(screen_framebuffer,
                                  true,
                                  Pathfinder::ColorF(0.3, 0.3, 0.3, 1.0));

    // Draw canvas to screen.
    texture_rect0->set_texture(canvas->get_dest_texture());
    texture_rect0->draw(cmd_buffer, screen_framebuffer);

    // Draw label to screen.
    texture_rect1->set_texture(label->canvas->get_dest_texture());
    texture_rect1->draw(cmd_buffer, screen_framebuffer);

    cmd_buffer->end_render_pass();

    cmd_buffer->submit();
}
