#ifndef PATHFINDER_GPU_QUEUE_VK_H
#define PATHFINDER_GPU_QUEUE_VK_H

#include "../queue.h"
#include "swap_chain.h"

namespace Pathfinder {

class QueueVk : public Queue {
    friend class WindowVk;
    friend class WindowBuilderVk;

public:
    void submit_and_wait(const std::shared_ptr<CommandEncoder> &encoder) override;

private:
    VkDevice vk_device_{};

    VkQueue vk_graphics_queue_{};

    VkQueue vk_present_queue_{};

public:
    QueueVk(VkDevice vk_device, VkQueue vk_graphics_queue, VkQueue vk_present_queue) {
        vk_device_ = vk_device;
        vk_graphics_queue_ = vk_graphics_queue;
        vk_present_queue_ = vk_present_queue;
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_QUEUE_VK_H
