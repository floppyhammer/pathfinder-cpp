#include "app.h"

/// Period to calculate average frame time, in seconds.
const float FRAME_TIME_PERIOD = 5;

App::App(const std::shared_ptr<Driver> &_driver,
         const std::vector<char> &svg_input,
         const std::vector<char> &img_input) {
    Logger::set_level(Logger::Level::INFO);

    driver = _driver;

    // Set up a canvas.
    canvas = std::make_shared<Canvas>(driver);

    // TEST: Clip path.
    if (false) {
        Path2d path;
        path.add_rect(RectF(Vec2F(0.0, 0.0), Vec2F(360.0, 360.0)));

        canvas->clip_path(path, FillRule::Winding);
    }

    // TEST: Draw image.
    if (true) {
        auto image_buffer = ImageBuffer::from_memory(img_input, false);
        if (image_buffer) {
            auto image = std::make_shared<Image>(image_buffer->get_size(), image_buffer->to_rgba_pixels());
            Vec2F pos = {100, 200};
            canvas->draw_image(image, RectF(pos, pos + image->size.to_f32()));
        }
    }

    // TEST: Minimal path.
    if (true) {
        Path2d path;
        path.move_to(260.0, 260.0);
        path.line_to(460.0, 260.0);
        path.line_to(460.0, 460.0);
        path.line_to(260.0, 460.0);
        path.close_path();

        // TEST: Shadow/Blur.
        if (false) {
            canvas->set_shadow_color(ColorU::red());
            canvas->set_shadow_blur(16);
        }

        // Set brush.
        canvas->set_line_width(10.0);
        canvas->set_stroke_paint(Paint::from_color(ColorU::red()));
        auto gradient = Gradient::linear(LineSegmentF({260.0, 260.0}, {460.0, 460.0}));
        gradient.add_color_stop(ColorU::red(), 0);
        gradient.add_color_stop(ColorU::transparent_black(), 1);
        gradient.add_color_stop(ColorU::blue(), 0.5);
        gradient.add_color_stop(ColorU::green(), 0.25);
        canvas->set_stroke_paint(Paint::from_gradient(gradient));

        canvas->stroke_path(path);
    }

    // TEST: Render target pattern.
    if (true) {
        auto render_target_size = Vec2I(400, 300);
        auto render_target_desc = RenderTargetDesc{render_target_size, "Sub render target"};
        auto render_target_id = canvas->get_scene()->push_render_target(render_target_desc);

        Path2d path;
        path.add_circle({200, 150}, 50);
        path.add_line({}, {200, 150});

        // Set brush.
        canvas->set_line_width(10.0);
        canvas->set_stroke_paint(Paint::from_color(ColorU::red()));
        canvas->stroke_path(path);

        canvas->get_scene()->pop_render_target();

        canvas->draw_render_target(render_target_id, {{}, render_target_size.to_f32()});
    }

    // TEST: Append SVG scene.
    if (true) {
        SvgScene svg_scene;
        svg_scene.load_from_string(std::string(svg_input.begin(), svg_input.end()), *canvas);

        // TEST: Replace scene.
        // canvas->set_scene(svg_scene.get_scene());

        // TEST: Append scene.
        canvas->get_scene()->append_scene(*svg_scene.get_scene(), Transform2());
    }

    // Timer.
    last_time = std::chrono::steady_clock::now();
}

void App::update() {
    // Timing.
    // ----------------------------------------
    frame_count++;

    const auto current_time = std::chrono::steady_clock::now();

    std::chrono::duration<double> duration = current_time - last_time;
    float elapsed_time = duration.count();

    if (elapsed_time > FRAME_TIME_PERIOD) {
        // Average frame time in ms.
        float average_frame_time = elapsed_time * 1000.f / (float)frame_count;

        // Show frame time.
        std::ostringstream string_stream;
        string_stream << "Frame time: " << round(average_frame_time * 100.f) * 0.01f << " ms";
        Logger::info(string_stream.str(), "Benchmark");

        frame_count = 0;
        last_time = current_time;
    }
    // ----------------------------------------

    canvas->draw();
}

void App::cleanup() {
    canvas.reset();
}
