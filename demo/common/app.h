#ifndef PATHFINDER_DEMO_APP_H
#define PATHFINDER_DEMO_APP_H

#include <chrono>

#include "pathfinder.h"
#include "texture_rect.h"

using namespace Pathfinder;

class App {
public:
    App(const std::shared_ptr<Pathfinder::Driver> &_driver,
        const Vec2I &window_size,
        const std::vector<char> &svg_input,
        const std::vector<char> &img_input);

    void update();

    void cleanup();

    void when_window_resized(Vec2I new_size) const;

    std::shared_ptr<Canvas> canvas;

private:
    std::shared_ptr<Driver> driver;

    std::chrono::time_point<std::chrono::steady_clock> last_time;
    uint32_t frame_count = 0;
};

#endif // PATHFINDER_DEMO_APP_H
