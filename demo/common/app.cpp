#include "app.h"

#include "../../src/common/logger.h"
#include "../../src/modules/vgui/servers/vector_server.h"
#include "../../src/gpu/platform.h"

App::App(int window_width,
         int window_height,
         std::vector<char> &area_lut_input,
         std::vector<char> &font_input,
         const std::string &p_svg_input) {
    // Set logger level.
    Pathfinder::Logger::set_level(Pathfinder::Logger::Level::DEBUG);

    Pathfinder::Platform::get_singleton().init(Pathfinder::DeviceType::GL4);
    Pathfinder::Platform::get_singleton().recreate_swap_chain(window_width, window_height);

    Pathfinder::VectorServer::get_singleton().init(window_width,
                                                   window_height,
                                                   area_lut_input);

    // Set up a canvas.
    canvas = std::make_shared<Pathfinder::Canvas>(window_width,
                                                  window_height,
                                                  area_lut_input);
    canvas->load_svg(p_svg_input);

    // Set up a text label.
    label = std::make_shared<Pathfinder::Label>();
    label->set_rect_size(128, 64);
    label->set_style(64, Pathfinder::ColorU::white(), 0, Pathfinder::ColorU::red());
    label->set_font(std::make_shared<Pathfinder::Font>(font_input));
    label->set_horizontal_alignment(Pathfinder::Alignment::Center);

    button = std::make_shared<Pathfinder::Button>();
    button->set_rect_size(128, 64);
    button->set_rect_position(400, 0);

    // Set viewport texture to a texture rect.
    texture_rect0 = std::make_shared<Pathfinder::TextureRect>(window_width, window_height);
    texture_rect1 = std::make_shared<Pathfinder::TextureRect>(window_width, window_height);

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

    // Server process.
    Pathfinder::VectorServer::get_singleton().canvas->clear();

    // Input.
    {
        auto queue = Pathfinder::InputServer::get_singleton().input_queue;
        button->input(queue);
    }

    // Update.
    {
        label->update();
    }

    // Draw.
    {
        canvas->draw();
        label->draw();
        button->draw();
    }

    // Server process.
    Pathfinder::VectorServer::get_singleton().canvas->draw();

    auto cmd_buffer = Pathfinder::Platform::get_singleton().device->create_command_buffer();

    auto framebuffer = Pathfinder::Platform::get_singleton().swap_chain->get_framebuffer(0);
    cmd_buffer->begin_render_pass(framebuffer,
                                  true,
                                  Pathfinder::ColorF(0.2, 0.2, 0.2, 1.0));

    // Draw canvas to screen.
    texture_rect0->set_texture(canvas->get_dest_texture());
    texture_rect0->draw(cmd_buffer, framebuffer->get_size());

    // Draw label to screen.
    texture_rect1->set_texture(Pathfinder::VectorServer::get_singleton().canvas->get_dest_texture());
    texture_rect1->draw(cmd_buffer, framebuffer->get_size());

    cmd_buffer->end_render_pass();

    cmd_buffer->submit();

    Pathfinder::InputServer::get_singleton().clear_queue();
}
