//
// Created by floppyhammer on 4/14/2022.
//

#include "panel.h"

#include "../../../common/math/basic.h"
#include "../../../common/math/mat4x4.h"
#include "../../../common/global_macros.h"
#include "../../../rendering/device.h"

namespace Pathfinder {
    void Panel::set_style_box(const StyleBox& p_style_box) {
        style_box = p_style_box;

        // Rebuild & draw the style box here.
    }

    StyleBox Panel::get_style_box() const {
        return style_box;
    }
}
