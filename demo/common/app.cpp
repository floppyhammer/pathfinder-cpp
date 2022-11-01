#include "app.h"

App::App(const std::shared_ptr<Pathfinder::Driver> &_driver,
         int window_width,
         int window_height,
         const std::vector<char> &svg_input) {
    // Set logger level.
    Pathfinder::Logger::set_level(Pathfinder::Logger::Level::INFO);

    driver = _driver;

    // Set up a canvas.
    canvas = std::make_shared<Pathfinder::Canvas>(driver);
    canvas->set_size({window_width, window_height});
    canvas->set_empty_dest_texture(window_width, window_height);

    // TEST: Clip path.
    if (false) {
        Pathfinder::Path2d path;
        path.add_rect(Pathfinder::RectF(Pathfinder::Vec2F(0.0, 0.0), Pathfinder::Vec2F(360.0, 360.0)));

        canvas->clip_path(path, Pathfinder::FillRule::Winding);
    }

    // TEST: Draw image.
    if (true) {
        Pathfinder::Image image;
        auto image_buffer = Pathfinder::ImageBuffer::from_file("../assets/test.png", false);
        image.size = {image_buffer->width, image_buffer->height};
        image.pixels = image_buffer->to_rgba_pixels();
        Pathfinder::Vec2F pos = {100, 200};
        canvas->draw_image(image, Pathfinder::RectF(pos, pos + image.size.to_f32()));
    }

    // TEST: Minimal path.
    if (true) {
        Pathfinder::Path2d path;
        path.move_to(260.0, 260.0);
        path.line_to(460.0, 260.0);
        path.line_to(460.0, 460.0);
        path.line_to(260.0, 460.0);
        path.close_path();

        // TEST: Shadow/Blur.
        if (false) {
            canvas->set_shadow_color(Pathfinder::ColorU::red());
            canvas->set_shadow_blur(16);
        }

        // Set brush.
        canvas->set_line_width(10.0);
        canvas->set_stroke_paint(Pathfinder::Paint::from_color(Pathfinder::ColorU::red()));
        auto gradient = Pathfinder::Gradient::linear(Pathfinder::LineSegmentF({260.0, 260.0}, {460.0, 460.0}));
        gradient.add_color_stop(Pathfinder::ColorU::red(), 0);
        gradient.add_color_stop(Pathfinder::ColorU::blue(), 1);
        canvas->set_stroke_paint(Pathfinder::Paint::from_gradient(gradient));

        canvas->stroke_path(path);
    }

    // TEST: Draw SVG.
    if (false) {
        Pathfinder::SvgScene svg_scene;
        svg_scene.load_from_memory(svg_input, *canvas);

        // TEST: Replace scene.
        //        canvas->set_scene(svg_scene.get_scene());

        // TEST: Append scene.
        canvas->get_scene()->append_scene(*svg_scene.get_scene(), Pathfinder::Transform2());
    }

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
