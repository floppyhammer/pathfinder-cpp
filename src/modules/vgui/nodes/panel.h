#ifndef PATHFINDER_SCENE_PANEL_H
#define PATHFINDER_SCENE_PANEL_H

#include "control.h"

#include "../resources/style_box.h"

#include <memory>

namespace Pathfinder {
    class Panel : public Control {
    public:
        void set_style_box(const StyleBox& p_style_box);

        StyleBox get_style_box() const;

    private:
        StyleBox style_box;
    };
}

#endif //PATHFINDER_SCENE_PANEL_H
