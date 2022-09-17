#ifndef PATHFINDER_DEMO_APP_H
#define PATHFINDER_DEMO_APP_H

#include "pathfinder.h"
#include "texture_rect.h"

#include <chrono>

class App {
public:
    App(const std::shared_ptr<Pathfinder::Driver> &p_driver,
        uint32_t window_width,
        uint32_t window_height,
        const std::vector<char> &p_svg_input);

    void update();

    void cleanup();

    std::shared_ptr<Pathfinder::Canvas> canvas;

private:
    std::shared_ptr<Pathfinder::Driver> driver;

    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> last_time;
    std::chrono::time_point<std::chrono::steady_clock> last_time_updated_fps;
};

#endif //PATHFINDER_DEMO_APP_H
