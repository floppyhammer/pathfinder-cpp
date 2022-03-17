//
// Created by chy on 2/24/2022.
//

#ifndef PATHFINDER_DEMO_APP_H
#define PATHFINDER_DEMO_APP_H

#include "../../src/rendering/viewport.h"
#include "../../src/scenes/gui/label.h"
#include "../../src/scenes/gui/texture_rect.h"
#include "../../src/modules/d3d9_d3d11/canvas.h"

#include <chrono>

class App {
public:
    App(int window_width,
        int window_height,
        std::vector<char> &area_lut_input,
        std::vector<char> &font_input,
        const std::string &p_svg_input);

    void loop();
    void cleanup();

private:
    std::shared_ptr<Pathfinder::Canvas> canvas;
    std::shared_ptr<Pathfinder::Label> label;
    std::shared_ptr<Pathfinder::TextureRect> texture_rect;
    std::shared_ptr<Pathfinder::Viewport> screen_viewport;

    std::string svg_input;

    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> last_time;
    std::chrono::time_point<std::chrono::steady_clock> last_time_updated_fps;
};

#endif //PATHFINDER_DEMO_APP_H
