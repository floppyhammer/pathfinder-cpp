#include "app.h"

using namespace Pathfinder;

/// Period to calculate average frame time, in seconds.
constexpr float FRAME_TIME_PERIOD = 5;

App::App(const std::shared_ptr<Device> &device,
         const std::shared_ptr<Queue> &queue,
         const Vec2I &canvas_size,
         const std::vector<char> &svg_input,
         const std::vector<char> &img_input) {
    Logger::set_default_level(Logger::Level::Info);

    device_ = device;
    queue_ = queue;

    // Set up a canvas.
    canvas_ = std::make_shared<Canvas>(canvas_size, device, queue, RenderLevel::D3d9);

    // TEST: View box clipping.
    if (true) {
        Path2d path;
        path.add_rect(RectF(Vec2F(400, 400), canvas_size.to_f32() + Vec2F(100)));

        canvas_->set_fill_paint(Paint::from_color(ColorU::red()));
        canvas_->fill_path(path, FillRule::Winding);
    }

    // TEST: Clip path.
    if (true) {
        Path2d path;
        path.add_rect(RectF(Vec2F(0.0, 0.0), Vec2F(360.0, 360.0)));

        canvas_->clip_path(path, FillRule::Winding);
    }

    // TEST: Draw image.
    if (true) {
        auto image_buffer = ImageBuffer::from_memory(img_input, false);
        if (image_buffer) {
            auto image = std::make_shared<Image>(image_buffer->get_size(), image_buffer->to_rgba_pixels());
            Vec2F pos = {10, 20};
            canvas_->draw_image(image, RectF(pos, pos + image->size.to_f32()));
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
            canvas_->set_shadow_color(ColorU::red());
            canvas_->set_shadow_blur(16);
        }

        // Set brush.
        canvas_->set_line_width(10.0);
        canvas_->set_stroke_paint(Paint::from_color(ColorU::red()));
        auto gradient = Gradient::linear(LineSegmentF({260.0, 260.0}, {460.0, 460.0}));
        gradient.add_color_stop(ColorU::red(), 0);
        gradient.add_color_stop(ColorU::transparent_black(), 1);
        gradient.add_color_stop(ColorU::blue(), 0.5);
        gradient.add_color_stop(ColorU::green(), 0.25);
        canvas_->set_stroke_paint(Paint::from_gradient(gradient));

        canvas_->stroke_path(path);
    }

    // TEST: Render a target pattern.
    if (true) {
        auto render_target_size = Vec2I(400, 300);
        auto render_target_desc = RenderTargetDesc{render_target_size, "sub render target"};
        auto render_target_id = canvas_->get_scene()->push_render_target(render_target_desc);

        Path2d path;
        path.add_circle({200, 150}, 50);
        path.add_line({}, {200, 150});

        // Set brush.
        canvas_->set_line_width(10.0);
        canvas_->set_stroke_paint(Paint::from_color(ColorU::red()));
        canvas_->stroke_path(path);

        canvas_->get_scene()->pop_render_target();

        canvas_->draw_render_target(render_target_id, {{}, render_target_size.to_f32()});
    }

    scene_0_ = canvas_->get_scene();

    // TEST: Append an SVG scene.
    if (true) {
        auto svg_scene = SvgScene(std::string(svg_input.begin(), svg_input.end()), *canvas_);

        // TEST: Replace with a scene.
        scene_1_ = svg_scene.get_scene();

        // TEST: Append a scene.
        //        canvas_->get_scene()->append_scene(*scene_1, Transform2::from_scale({1.0, 1.0}));
    }

    // Timer.
    last_time_ = std::chrono::steady_clock::now();
}

void App::update() {
    // Timing.
    // ----------------------------------------
    frame_count_++;

    const auto current_time = std::chrono::steady_clock::now();

    std::chrono::duration<double> duration = current_time - last_time_;
    double elapsed_time = duration.count();

    if (elapsed_time > FRAME_TIME_PERIOD) {
        // Average frame time in ms.
        double average_frame_time = elapsed_time * 1000.f / (float)frame_count_;

        // Show frame time.
        std::ostringstream string_stream;
        string_stream << "Frame time: " << round(average_frame_time * 100.f) * 0.01f << " ms";
        Logger::info(string_stream.str(), "Benchmark");

        frame_count_ = 0;
        last_time_ = current_time;
    }
    // ----------------------------------------

    canvas_->set_scene(scene_0_);
    canvas_->draw(true);

    canvas_->set_scene(scene_1_);
    canvas_->draw(false);
}

void App::destroy() {
    canvas_.reset();
}
