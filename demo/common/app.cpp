#include "app.h"

App::App(const std::shared_ptr<Pathfinder::Driver> &p_driver,
         uint32_t window_width,
         uint32_t window_height,
         const std::vector<char> &p_svg_input) {
    // Set logger level.
    Pathfinder::Logger::set_level(Pathfinder::Logger::Level::DEBUG);

    driver = p_driver;

    // Set up a canvas.
    canvas = std::make_shared<Pathfinder::Canvas>(driver);
    canvas->set_empty_scene({0, 0, (float) window_width, (float) window_height});
    canvas->set_empty_dest_texture(window_width, window_height);
    canvas->load_svg(p_svg_input);

    // Timers.
    start_time = std::chrono::steady_clock::now();
    last_time = start_time;
    last_time_updated_fps = start_time;
}

void App::update() {
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
}

void App::cleanup() {
    canvas.reset();
}
