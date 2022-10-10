#include "app.h"

App::App(const std::shared_ptr<Pathfinder::Driver> &p_driver,
         uint32_t window_width,
         uint32_t window_height,
         const std::vector<char> &p_svg_input) {
    // Set logger level.
    Pathfinder::Logger::set_level(Pathfinder::Logger::Level::INFO);

    driver = p_driver;

    // Set up a canvas.
    canvas = std::make_shared<Pathfinder::Canvas>(driver);
    canvas->set_empty_scene({0, 0, (float)window_width, (float)window_height});
    canvas->set_empty_dest_texture(window_width, window_height);
    canvas->load_svg(p_svg_input);

    // Timers.
    last_time = std::chrono::steady_clock::now();
    last_time_printed_fps = last_time;
}

void App::update() {
    // Timing.
    // ----------------------------------------
    const auto current_time = std::chrono::steady_clock::now();

    std::chrono::duration<double> duration = current_time - last_time_printed_fps;

    if (duration.count() > 1) {
        last_time_printed_fps = current_time;

        // Time between frames in ms.
        duration = current_time - last_time;
        float delta = (float)duration.count() * 1000.f;

        // Show frame time.
        std::ostringstream string_stream;
        string_stream << "Frame time: " << round(delta * 10.f) * 0.1f << " s\n";
        Pathfinder::Logger::info(string_stream.str(), "Benchmark");
    }

    last_time = current_time;
    // ----------------------------------------

    canvas->draw();
}

void App::cleanup() {
    canvas.reset();
}
