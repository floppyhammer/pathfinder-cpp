//
// Created by floppyhammer on 2021/12/30.
//

#include "control.h"

namespace Pathfinder {
    Control::Control() {
        type = NodeType::Control;
    }

    void Control::set_rect_position(float x, float y) {
        rect_position.x = x;
        rect_position.y = y;
    }

    Vec2<float> Control::get_rect_position() const {
        return rect_position;
    }

    void Control::set_rect_size(float w, float h) {
        rect_size.x = w;
        rect_size.y = h;
    }

    Vec2<float> Control::get_rect_size() const {
        return rect_size;
    }

    void Control::set_rect_scale(float x, float y) {
        rect_scale.x = x;
        rect_scale.y = y;
    }

    Vec2<float> Control::get_rect_scale() const {
        return rect_scale;
    }

    void Control::set_rect_rotation(float r) {
        rect_rotation = r;
    }

    float Control::get_rect_rotation() const {
        return rect_rotation;
    }

    void Control::set_rect_pivot_offset(float x, float y) {
        rect_pivot_offset.x = x;
        rect_pivot_offset.y = y;
    }

    Vec2<float> Control::get_rect_pivot_offset() const {
        return rect_pivot_offset;
    }

    void Control::update() {

    }

    void Control::draw() {
        auto canvas = VectorServer::get_singleton().canvas;

        // Rebuild & draw the style box.
        auto style_box_shape = Shape();
        style_box_shape.add_rect(Rect<float>(Vec2<float>(), rect_size), style_box.corner_radius);

        canvas->set_fill_paint(Paint::from_color(style_box.bg_color));
        canvas->fill_shape(style_box_shape, FillRule::Winding);

        if (style_box.border_width > 0) {
            canvas->set_stroke_paint(Paint::from_color(style_box.border_color));
            canvas->set_line_width(style_box.border_width);
            canvas->stroke_shape(style_box_shape);
        }
    }

    void Control::set_style_box(const StyleBox& p_style_box) {
        style_box = p_style_box;
    }

    StyleBox Control::get_style_box() const {
        return style_box;
    }
}
