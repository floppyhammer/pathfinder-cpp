#ifndef PATHFINDER_RESOURCE_STYLE_BOX_H
#define PATHFINDER_RESOURCE_STYLE_BOX_H

#include "../../d3dx/data/shape.h"

namespace Pathfinder {
    struct StyleBox {
        ColorU bg_color = ColorU(30, 30, 30, 255);
        bool draw_center = true;

        ColorU border_color;
        float border_width;

        float corner_radius = 8;

        float margin = 0;

        ColorU shadow_color;
        float shadow_size;
        Vec2<float> shadow_offset;
    };
}

#endif //PATHFINDER_RESOURCE_STYLE_BOX_H
