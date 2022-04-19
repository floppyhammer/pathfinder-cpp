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

    Vec2<float> Control::calculate_minimum_size() const {
        return {};
    }

    void Control::input(std::vector<InputEvent> &input_queue) {

    }

    void Control::update() {

    }

    void Control::draw() {
    }

    Vec2<float> Control::get_minimum_size() const {
        return minimum_size;
    }
}
