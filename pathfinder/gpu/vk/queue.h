#pragma once

#include "../queue.h"
#include "swap_chain.h"

namespace Pathfinder {

class QueueVk : public Queue {
    friend class WindowVk;
    friend class WindowBuilderVk;

public:
    void submit(const std::shared_ptr<CommandEncoder> &encoder, const std::shared_ptr<Fence> &fence) override;

    void begin_frame(uint32_t current_frame_index) override;

private:
    VkDevice vk_device_{};

    VkQueue vk_graphics_queue_{};

    VkQueue vk_present_queue_{};

    std::vector<std::vector<std::shared_ptr<CommandEncoder>>> encoders_in_flight_;

    uint32_t frames_in_flight_{};

public:
    QueueVk(VkDevice vk_device, VkQueue vk_graphics_queue, VkQueue vk_present_queue, uint32_t frames_in_flight) {
        vk_device_ = vk_device;
        vk_graphics_queue_ = vk_graphics_queue;
        vk_present_queue_ = vk_present_queue;
        frames_in_flight_ = frames_in_flight;
        encoders_in_flight_.resize(frames_in_flight);
    }
};

} // namespace Pathfinder
