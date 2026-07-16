#include "queue.h"

#include "fence.h"

namespace Pathfinder {

void QueueVk::submit(const std::shared_ptr<CommandEncoder> &encoder, const std::shared_ptr<Fence> &fence) {
    if (encoder->submitted_) {
        Logger::error("Attempted to submit an encoder that's already been submitted!");
        return;
    }

    // Mark the encoder as submitted.
    encoder->submitted_ = true;

    if (!encoder->prepare()) {
        return;
    }

    auto encoder_vk = (CommandEncoderVk *)encoder.get();

    // Submit the command buffer to the graphics queue.
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &encoder_vk->vk_command_buffer_;

    if (fence) {
        auto fence_vk = (FenceVk *)fence.get();

        vkQueueSubmit(vk_graphics_queue_, 1, &submit_info, fence_vk->fence);

        fence_vk->wait();
    } else {
        vkQueueSubmit(vk_graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);

        // If no fence is provided, but there are callbacks (e.g. from read_buffer),
        // we must ensure the GPU is finished before invoking them.
        // A simple way is to wait for the queue to be idle.
        if (!encoder->callbacks_.empty()) {
            vkQueueWaitIdle(vk_graphics_queue_);
        } else {
            encoders_in_flight_[current_frame_index_ % frames_in_flight_].push_back(encoder);
        }
    }

    encoder->invoke_callbacks();
}

void QueueVk::begin_frame(uint32_t current_frame_index) {
    current_frame_index_ = current_frame_index;
    encoders_in_flight_[current_frame_index_ % frames_in_flight_].clear();
}

} // namespace Pathfinder
