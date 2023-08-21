#ifndef PATHFINDER_DEMO_APP_H
#define PATHFINDER_DEMO_APP_H

#include <chrono>

#include "pathfinder.h"
#include "texture_rect.h"

using namespace Pathfinder;

class App {
public:
    App(const std::shared_ptr<Pathfinder::Device> &device,
        const std::shared_ptr<Queue> &queue,
        const Vec2I &canvas_size,
        const std::vector<char> &svg_input,
        const std::vector<char> &img_input);

    void update();

    void cleanup();

    std::shared_ptr<Canvas> canvas;

    std::shared_ptr<Scene> scene_0, scene_1;

    std::shared_ptr<Device> device;
    std::shared_ptr<Queue> queue;

private:
    std::chrono::time_point<std::chrono::steady_clock> last_time;
    uint32_t frame_count = 0;
};

#endif // PATHFINDER_DEMO_APP_H
