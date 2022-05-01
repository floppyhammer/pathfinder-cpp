#ifndef PATHFINDER_DEMO_APP_H
#define PATHFINDER_DEMO_APP_H

#include "pathfinder.h"
#include "texture_rect.h"

#include <chrono>

class App {
public:
    App(const std::shared_ptr<Pathfinder::Driver> &p_driver,
        const std::shared_ptr<Pathfinder::SwapChain> &swap_chain,
        uint32_t window_width,
        uint32_t window_height,
        std::vector<char> &area_lut_input,
        std::vector<char> &font_input,
        const std::string &p_svg_input);

    void loop(const std::shared_ptr<Pathfinder::SwapChain> &swap_chain);

private:
    std::shared_ptr<Pathfinder::Driver> driver;

    std::shared_ptr<Pathfinder::Canvas> canvas;

    std::shared_ptr<TextureRect> texture_rect;

    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> last_time;
    std::chrono::time_point<std::chrono::steady_clock> last_time_updated_fps;
};

#endif //PATHFINDER_DEMO_APP_H
