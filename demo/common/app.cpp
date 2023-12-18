#include "app.h"

using namespace Pathfinder;

/// Period to calculate average frame time, in seconds.
const float FRAME_TIME_PERIOD = 5;

App::App(const std::shared_ptr<Device> &_device,
         const std::shared_ptr<Queue> &_queue,
         const Vec2I &canvas_size,
         const std::vector<char> &svg_input,
         const std::vector<char> &img_input) {
    Logger::set_level(Logger::Level::Info);

    device = _device;
    queue = _queue;

    // Set up a canvas.
    canvas = std::make_shared<Canvas>(canvas_size, device, queue, RenderLevel::Dx9);

    // TEST: View box clipping.
    if (true) {
        Path2d path;
        path.add_rect(RectF(Vec2F(400, 400), canvas_size.to_f32() + Vec2F(100)));

        canvas->set_fill_paint(Paint::from_color(ColorU::red()));
        canvas->fill_path(path, FillRule::Winding);
    }

    // TEST: Clip path.
    if (true) {
        Path2d path;
        path.add_rect(RectF(Vec2F(0.0, 0.0), Vec2F(360.0, 360.0)));

        canvas->clip_path(path, FillRule::Winding);
    }

    // TEST: Draw image.
    if (true) {
        auto image_buffer = ImageBuffer::from_memory(img_input, false);
        if (image_buffer) {
            auto image = std::make_shared<Image>(image_buffer->get_size(), image_buffer->to_rgba_pixels());
            Vec2F pos = {10, 20};
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

    scene_0 = canvas->get_scene();

    // TEST: Append SVG scene.
    if (true) {
        auto svg_scene = SvgScene(std::string(svg_input.begin(), svg_input.end()), *canvas);

        // TEST: Replace scene.
        scene_1 = svg_scene.get_scene();

        // TEST: Append scene.
        //        canvas->get_scene()->append_scene(*scene_1, Transform2::from_scale({1.0, 1.0}));
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
    double elapsed_time = duration.count();

    if (elapsed_time > FRAME_TIME_PERIOD) {
        // Average frame time in ms.
        double average_frame_time = elapsed_time * 1000.f / (float)frame_count;

        // Show frame time.
        std::ostringstream string_stream;
        string_stream << "Frame time: " << round(average_frame_time * 100.f) * 0.01f << " ms";
        Logger::info(string_stream.str(), "Benchmark");

        frame_count = 0;
        last_time = current_time;
    }
    // ----------------------------------------

    canvas->set_scene(scene_0);
    canvas->draw(true);

    canvas->set_scene(scene_1);
    canvas->draw(false);
}

void App::destroy() {
    canvas.reset();
}
