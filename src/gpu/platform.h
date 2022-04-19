#ifndef PATHFINDER_GPU_PLATFORM_H
#define PATHFINDER_GPU_PLATFORM_H

#include "gl/device.h"
#include "vk/device.h"

namespace Pathfinder {
    class Platform {
    public:
        inline void init(DeviceType device_type) {
            switch (device_type) {
#ifdef PATHFINDER_USE_VULKAN
                case DeviceType::Vulkan: {


                    device = std::make_shared<Pathfinder::DeviceVk>(vk_device, vk_physical_device);
                }
                    break;
#endif
                default: {
                    device = std::make_shared<Pathfinder::DeviceGl>();
                }
                    break;
            }
        }

        inline void recreate_swap_chain(uint32_t p_width, uint32_t p_height) {
            swap_chain = device->create_swap_chain(p_width, p_height);
        }

        static Platform &get_singleton() {
            static Platform singleton;
            return singleton;
        }

        std::shared_ptr<Device> device;
        std::shared_ptr<SwapChain> swap_chain;
    };
}

#endif //PATHFINDER_GPU_PLATFORM_H
