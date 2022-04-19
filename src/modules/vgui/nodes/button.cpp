#include "button.h"

namespace Pathfinder {
    Button::Button() {
        type = NodeType::Button;

        theme_hovered.border_color = ColorU(200, 200, 200, 255);

        theme_pressed.bg_color = ColorU(70, 70, 70, 255);
        theme_pressed.border_color = ColorU(200, 200, 200, 255);
    }

    void Button::input(std::vector<InputEvent> &input_queue) {
        for (auto &event : input_queue) {
            if (event.type == InputEventType::MouseMotion) {
                auto args = event.args.mouse_motion;

                if (Rect<float>(rect_position, rect_position + rect_size).contains_point(args.position)) {
                    hovered = true;
                } else {
                    hovered = false;
                }
            }

            if (event.type == InputEventType::MouseButton) {
                auto args = event.args.mouse_button;

                if (Rect<float>(rect_position, rect_position + rect_size).contains_point(args.position)) {
                    pressed = args.pressed;
                }
            }
        }
    }

    void Button::update() {

    }

    void Button::draw() {
        auto canvas = VectorServer::get_singleton().canvas;

        StyleBox active_style_box;
        if (hovered) {
            active_style_box = pressed ? theme_pressed : theme_hovered;
        } else {
            active_style_box = theme_normal;
        }

        // Rebuild & draw the style box.
        auto style_box_shape = Shape();
        style_box_shape.add_rect(Rect<float>(rect_position, rect_position + rect_size), active_style_box.corner_radius);

        canvas->set_fill_paint(Paint::from_color(active_style_box.bg_color));
        canvas->fill_shape(style_box_shape, FillRule::Winding);

        if (active_style_box.border_width > 0) {
            canvas->set_stroke_paint(Paint::from_color(active_style_box.border_color));
            canvas->set_line_width(active_style_box.border_width);
            canvas->stroke_shape(style_box_shape);
        }
    }
}
