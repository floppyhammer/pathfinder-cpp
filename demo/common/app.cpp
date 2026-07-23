#include "app.h"

using namespace Pathfinder;

/// Period to calculate average frame time, in seconds.
constexpr float FRAME_TIME_PERIOD = 5;

App::App(const std::shared_ptr<Device> &device,
         const std::shared_ptr<Queue> &queue,
         const Vec2I &canvas_size,
         const std::vector<char> &svg_input,
         const std::vector<char> &img_input) {
    Logger::set_global_level(Logger::Level::Info);

    device_ = device;
    queue_ = queue;

    // Set up a canvas.
    canvas_ = std::make_shared<Canvas>(canvas_size, device, queue, RenderMode::Hybrid);

    // Test: view box clipping.
    if (true) {
        Path2d path;
        path.add_rect(RectF(Vec2F(400, 400), canvas_size.to_f32() + Vec2F(100)));

        canvas_->set_fill_paint(Paint::from_color(ColorU::red()));
        canvas_->fill_path(path, FillRule::Winding);
    }

    // Test: clip path.
    if (true) {
        Path2d path;
        path.add_circle(Vec2F(180.0, 180.0), 180);
        canvas_->clip_path(path, FillRule::Winding);
    }

    // Test: shadow/blur.
    if (true) {
        canvas_->set_shadow_color(ColorU::white());
        canvas_->set_shadow_blur(16);
        canvas_->set_shadow_offset({0, 0});
        canvas_->set_shadow_strength(1.0);
    }

    // Test: draw image.
    if (true) {
        auto image_buffer = ImageBuffer::from_memory(img_input, false);
        if (image_buffer) {
            auto image = std::make_shared<Image>(image_buffer->get_size(), image_buffer->to_rgba_pixels());
            Vec2F pos = {10, 20};
            canvas_->draw_image(image, RectF(pos, pos + image->size.to_f32()));
        }
    }

    // Test: minimal path.
    if (true) {
        Path2d path;
        path.move_to(260.0, 260.0);
        path.line_to(460.0, 260.0);
        path.line_to(460.0, 460.0);
        path.line_to(260.0, 460.0);
        path.close_path();

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

    // Test: draw a target pattern.
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

        auto pos = Vec2F(100, 50);

        canvas_->draw_render_target(render_target_id, {pos, pos + render_target_size.to_f32()});
    }

    scene_0_ = canvas_->get_scene();

    // Test: append an SVG scene.
    if (true) {
        auto svg_scene = SvgScene(std::string(svg_input.begin(), svg_input.end()), *canvas_);

        // Test: replace with a scene.
        scene_1_ = svg_scene.get_scene();

        // Test: append a scene.
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
        Logger::info(string_stream.str(), "Pathfinder");

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
