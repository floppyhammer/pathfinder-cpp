#ifndef PATHFINDER_DEMO_APP_H
#define PATHFINDER_DEMO_APP_H

#include <pathfinder/prelude.h>

#include <chrono>

#include "texture_rect.h"

class App {
public:
    App(const std::shared_ptr<Pathfinder::Device> &device,
        const std::shared_ptr<Pathfinder::Queue> &queue,
        const Pathfinder::Vec2I &canvas_size,
        const std::vector<char> &svg_input,
        const std::vector<char> &img_input);

    void update();

    void destroy();

    std::shared_ptr<Pathfinder::Canvas> canvas;

    std::shared_ptr<Pathfinder::Scene> scene_0, scene_1;

    std::shared_ptr<Pathfinder::Device> device;
    std::shared_ptr<Pathfinder::Queue> queue;

private:
    std::chrono::time_point<std::chrono::steady_clock> last_time;
    uint32_t frame_count = 0;
};

#endif // PATHFINDER_DEMO_APP_H
