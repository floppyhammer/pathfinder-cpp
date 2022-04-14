#ifndef PATHFINDER_RESOURCE_STYLE_BOX_H
#define PATHFINDER_RESOURCE_STYLE_BOX_H

#include "../../d3dx/data/shape.h"

namespace Pathfinder {
    class StyleBox {
    public:

    private:
        ColorF bg_color;
        bool draw_center = true;

        ColorF border_color;
        Rect<float> border_width;

        Rect<float> corner_radius;

        float margin = 0;

        ColorF shadow_color;
        float shadow_size;
        Vec2<float> shadow_offset;

    private:
        Shape fill;
        Shape stroke;
    };
}

#endif //PATHFINDER_RESOURCE_STYLE_BOX_H
