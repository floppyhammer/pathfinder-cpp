#include "queue.h"

namespace Pathfinder {

void QueueVk::submit_and_wait(std::shared_ptr<CommandEncoder> encoder) {
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
}

void QueueVk::submit(std::shared_ptr<CommandEncoder> encoder, std::shared_ptr<SwapChain> surface) {
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
}

} // namespace Pathfinder
