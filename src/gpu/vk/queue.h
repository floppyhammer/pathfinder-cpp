#ifndef PATHFINDER_GPU_QUEUE_VK_H
#define PATHFINDER_GPU_QUEUE_VK_H

#include <vector>

#include "../queue.h"
#include "swap_chain.h"

#ifdef PATHFINDER_USE_VULKAN

    #if defined(WIN32) || defined(__linux__) || defined(__APPLE__)

namespace Pathfinder {

class QueueVk : public Queue {
    friend class WindowVk;

public:
    QueueVk(VkDevice _device, VkQueue _graphics_queue, VkQueue _present_queue) {
        device = _device;
        graphics_queue = _graphics_queue;
        present_queue = _present_queue;
    }

    void submit_and_wait(std::shared_ptr<CommandEncoder> encoder) override {
        if (encoder->submitted) {
            Logger::error("Attempted to submit an encoder that's already been submitted!");
            return;
        }

        encoder->finish();

        auto encoder_vk = (CommandEncoderVk *)encoder.get();

        // Submit the command buffer to the graphics queue.
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &encoder_vk->vk_command_buffer;

        vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);

        // Wait for the queue to finish commands.
        vkQueueWaitIdle(graphics_queue);

        for (auto &callback : encoder->callbacks) {
            callback();
        }

        encoder->callbacks.clear();

        // Mark the encoder as submitted.
        encoder->submitted = true;

        encoder_vk->free();
    };

    void submit(std::shared_ptr<CommandEncoder> encoder, std::shared_ptr<SwapChain> surface) override {
        #ifndef ANDROID
        if (encoder_of_last_frame) {
            dynamic_cast<CommandEncoderVk *>(encoder_of_last_frame.get())->free();
            encoder_of_last_frame = nullptr;
        }

        if (encoder->submitted) {
            Logger::error("Attempted to submit an encoder that's already been submitted!");
            return;
        }

        encoder->finish();

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
};

} // namespace Pathfinder

    #endif

#endif

#endif // PATHFINDER_GPU_QUEUE_VK_H
