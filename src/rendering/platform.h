#ifndef PATHFINDER_PLATFORM_H
#define PATHFINDER_PLATFORM_H

#include "gl/device.h"

namespace Pathfinder {
    class Platform {
    public:
        Platform() {
            device = std::make_shared<Pathfinder::DeviceGl>();
        }

        static Platform &get_singleton() {
            static Platform singleton;
            return singleton;
        }

        std::shared_ptr<Device> device;
    };
}

#endif //PATHFINDER_PLATFORM_H
