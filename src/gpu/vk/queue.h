#ifndef PATHFINDER_GPU_QUEUE_VK_H
#define PATHFINDER_GPU_QUEUE_VK_H

#include <vector>

#include "../queue.h"
#include "swap_chain.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class QueueVk : public Queue {
    friend class WindowVk;
    friend class WindowBuilderVk;

public:
    void submit_and_wait(std::shared_ptr<CommandEncoder> encoder) override {
        if (encoder->submitted) {
            Logger::error("Attempted to submit an encoder that's already been submitted!");
            return;
        }

        // Mark the encoder as submitted.
        encoder->submitted = true;

        if (!encoder->finish()) {
            return;
        }

        auto encoder_vk = (CommandEncoderVk *)encoder.get();

        // Submit the command buffer to the graphics queue.
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &encoder_vk->vk_command_buffer;

        vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);

        // Wait for the queue to finish commands.
        vkQueueWaitIdle(graphics_queue);
    };

    void submit(std::shared_ptr<CommandEncoder> encoder, std::shared_ptr<SwapChain> surface) override {
    #ifndef ANDROID
        // Cleanup last encoder.
        if (encoder_of_last_frame) {
            encoder_of_last_frame = nullptr;
        }

        if (encoder->submitted) {
            Logger::error("Attempted to submit an encoder that's already been submitted!");
            return;
        }

        encoder->submitted = true;

        if (!encoder->finish()) {
            return;
        }

        auto surface_vk = (SwapChainVk *)surface.get();

        surface_vk->flush(encoder);

        encoder_of_last_frame = encoder;
    #endif
    };

private:
    VkDevice device{};

    VkQueue graphics_queue{};

    VkQueue present_queue{};

    std::shared_ptr<CommandEncoder> encoder_of_last_frame;

private:
    QueueVk(VkDevice _device, VkQueue _graphics_queue, VkQueue _present_queue) {
        device = _device;
        graphics_queue = _graphics_queue;
        present_queue = _present_queue;
    }
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_QUEUE_VK_H
