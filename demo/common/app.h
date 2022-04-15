//
// Created by floppyhammer on 2/24/2022.
//

#ifndef PATHFINDER_DEMO_APP_H
#define PATHFINDER_DEMO_APP_H

#include "../../src/rendering/gl/framebuffer.h"
#include "../../src/modules/d3dx/canvas.h"
#include "../../src/modules/vgui/nodes/label.h"
#include "../../src/modules/vgui/nodes/texture_rect.h"
#include "../../src/rendering/gl/device.h"

#include <chrono>

class App {
public:
    App(int window_width,
        int window_height,
        std::vector<char> &area_lut_input,
        std::vector<char> &font_input,
        const std::string &p_svg_input);

    void loop();

private:
    std::shared_ptr<Pathfinder::Canvas> canvas;
    std::shared_ptr<Pathfinder::Label> label;
    std::shared_ptr<Pathfinder::TextureRect> texture_rect0, texture_rect1;
    std::shared_ptr<Pathfinder::Framebuffer> screen_framebuffer;

    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> last_time;
    std::chrono::time_point<std::chrono::steady_clock> last_time_updated_fps;
};

#endif //PATHFINDER_DEMO_APP_H
