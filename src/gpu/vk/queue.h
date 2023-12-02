#ifndef PATHFINDER_GPU_QUEUE_VK_H
#define PATHFINDER_GPU_QUEUE_VK_H

#include <vector>

#include "../queue.h"
#include "swap_chain.h"

namespace Pathfinder {

class QueueVk : public Queue {
    friend class WindowVk;
    friend class WindowBuilderVk;

public:
    void submit_and_wait(std::shared_ptr<CommandEncoder> encoder) override;

    void submit(std::shared_ptr<CommandEncoder> encoder, std::shared_ptr<SwapChain> surface) override;

private:
    VkDevice device{};

    VkQueue graphics_queue{};

    VkQueue present_queue{};

    std::shared_ptr<CommandEncoder> encoder_of_last_frame;

public:
    QueueVk(VkDevice _device, VkQueue _graphics_queue, VkQueue _present_queue) {
        device = _device;
        graphics_queue = _graphics_queue;
        present_queue = _present_queue;
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_QUEUE_VK_H
