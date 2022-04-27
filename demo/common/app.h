#ifndef PATHFINDER_DEMO_APP_H
#define PATHFINDER_DEMO_APP_H

#include "../../src/gpu/driver.h"
#include "../../src/gpu/framebuffer.h"
#include "../../src/gpu/swap_chain.h"
#include "../../src/modules/d3dx/canvas.h"
#include "../../src/modules/vgui/nodes/label.h"
#include "../../src/modules/vgui/nodes/button.h"
#include "../../src/modules/vgui/nodes/texture_rect.h"
#include "../../src/modules/vgui/servers/input_server.h"
#include "../../src/common/global_macros.h"
#include "../../src/common/io.h"
#include <chrono>

class App {
public:
    App(const std::shared_ptr<Pathfinder::Driver> &p_driver,
        uint32_t window_width,
        uint32_t window_height,
        std::vector<char> &area_lut_input,
        std::vector<char> &font_input,
        const std::string &p_svg_input);

    void loop(const std::shared_ptr<Pathfinder::SwapChain> &swap_chain);

private:
    std::shared_ptr<Pathfinder::Driver> driver;

    std::shared_ptr<Pathfinder::Canvas> canvas;
    std::shared_ptr<Pathfinder::Label> label;
    std::shared_ptr<Pathfinder::TextureRect> texture_rect0, texture_rect1;
    std::shared_ptr<Pathfinder::Button> button;

    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> last_time;
    std::chrono::time_point<std::chrono::steady_clock> last_time_updated_fps;
};

#endif //PATHFINDER_DEMO_APP_H
