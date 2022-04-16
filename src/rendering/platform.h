#ifndef PATHFINDER_PLATFORM_H
#define PATHFINDER_PLATFORM_H

#include "data.h"
#include "gl/device.h"
#include "vk/device.h"

namespace Pathfinder {
    class Platform {
    public:
        inline void init(DeviceType device_type) {
            switch (device_type) {
#ifdef PATHFINDER_USE_VULKAN
                case DeviceType::Vulkan: {
                    device = std::make_shared<Pathfinder::DeviceVk>();
                }
                    break;
#endif
                default: {
                    device = std::make_shared<Pathfinder::DeviceGl>();
                }
                    break;
            }
        }

        static Platform &get_singleton() {
            static Platform singleton;
            return singleton;
        }

        std::shared_ptr<Device> device;
    };
}

#endif //PATHFINDER_PLATFORM_H
