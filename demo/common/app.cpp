//
// Created by floppyhammer on 2/24/2022.
//

#include "app.h"

#include "../../src/common/global_macros.h"
#include "../../src/common/logger.h"

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
    label = std::make_shared<Pathfinder::Label>(window_width,
                                                window_height,
                                                reinterpret_cast<std::vector<unsigned char> &>(area_lut_input));
    label->set_rect_size((float) window_width, (float) window_width);
    label->set_style(64, Pathfinder::ColorU::white(), 0, Pathfinder::ColorU::red());
    label->set_font(std::make_shared<Pathfinder::Font>(font_input));

    // Create a screen viewport.
    screen_viewport = std::make_shared<Pathfinder::Viewport>(window_width, window_height);
    screen_viewport->set_clear_color(Pathfinder::ColorF(0.3, 0.3, 0.3, 1.0));

    // Set viewport texture to a texture rect.
    texture_rect = std::make_shared<Pathfinder::TextureRect>(window_width, window_height);

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

    // Set render target to screen. Clear screen.
    screen_viewport->use();
    screen_viewport->clear();

    // Draw canvas to screen.
    texture_rect->set_texture(canvas->get_dest_texture());
    texture_rect->draw();

    // Draw label to screen.
    texture_rect->set_texture(label->canvas->get_dest_texture());
    texture_rect->draw();
}
