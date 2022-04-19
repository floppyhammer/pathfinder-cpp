#ifndef PATHFINDER_BUTTON_H
#define PATHFINDER_BUTTON_H

#include "control.h"

namespace Pathfinder {
    class Button : public Control {
    public:
        Button();

        bool pressed = false;
        bool hovered = false;

        void input(std::vector<InputEvent> &input_queue) override;

        void update() override;

        void draw() override;

    protected:
        StyleBox theme_normal, theme_hovered, theme_pressed;
    };
}

#endif //PATHFINDER_BUTTON_H
