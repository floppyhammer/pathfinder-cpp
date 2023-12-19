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

    std::shared_ptr<Pathfinder::Canvas> canvas_;

    std::shared_ptr<Pathfinder::Scene> scene_0_, scene_1_;

    std::shared_ptr<Pathfinder::Device> device_;
    std::shared_ptr<Pathfinder::Queue> queue_;

private:
    std::chrono::time_point<std::chrono::steady_clock> last_time_;
    uint32_t frame_count_ = 0;
};

#endif // PATHFINDER_DEMO_APP_H
