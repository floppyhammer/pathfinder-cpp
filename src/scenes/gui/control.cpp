//
// Created by chy on 2021/12/30.
//

#include "control.h"

namespace Pathfinder {
    void Control::set_rect_position(float x, float y) {
        rect_position.x = x;
        rect_position.y = y;
    }

    void Control::set_rect_size(float w, float h) {
        rect_size.x = w;
        rect_size.y = h;
    }

    void Control::set_rect_scale(float x, float y) {
        rect_scale.x = x;
        rect_scale.y = y;
    }

    void Control::set_rect_rotation(float r) {
        rect_rotation = r;
    }

    void Control::set_rect_pivot_offset(float x, float y) {
        rect_pivot_offset.x = x;
        rect_pivot_offset.y = y;
    }
}
