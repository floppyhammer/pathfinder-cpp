#ifndef PATHFINDER_DEMO_APP_H
#define PATHFINDER_DEMO_APP_H

#include <chrono>

#include "../../src/pathfinder.h"
#include "texture_rect.h"

class App {
public:
    App(const std::shared_ptr<Pathfinder::Driver> &_driver,
        int window_width,
        int window_height,
        const std::vector<char> &svg_input);

    void update();

    void cleanup();

    std::shared_ptr<Pathfinder::Canvas> canvas;

private:
    std::shared_ptr<Pathfinder::Driver> driver;

    std::chrono::time_point<std::chrono::steady_clock> last_time;
    std::chrono::time_point<std::chrono::steady_clock> last_time_printed_fps;
};

#endif // PATHFINDER_DEMO_APP_H
