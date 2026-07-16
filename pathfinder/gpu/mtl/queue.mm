#include "queue.h"

#include "command_encoder.h"

namespace Pathfinder {

void QueueMtl::submit(const std::shared_ptr<CommandEncoder>& encoder, const std::shared_ptr<Fence>& fence) {
    auto encoder_mtl = (CommandEncoderMtl*)encoder.get();

    @autoreleasepool {
        // Wait for a frame to become available.
        dispatch_semaphore_wait(in_flight_semaphore_, DISPATCH_TIME_FOREVER);

        encoder_mtl->prepare();

        auto mtl_cmd_buffer = encoder_mtl->get_handle();

        // We must create a local shared_ptr copy to ensure the block captures the object by value,
        // rather than capturing the reference to the parameter which will go out of scope.
        auto encoder_captured = encoder;
        auto semaphore_captured = in_flight_semaphore_;

        [mtl_cmd_buffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
            encoder_captured->invoke_callbacks();

            // Signal that the frame is complete.
            dispatch_semaphore_signal(semaphore_captured);
        }];

        [mtl_cmd_buffer commit];

        // If fence is provided, we should probably handle it.
        // Metal doesn't have a direct equivalent to Vulkan's fence for CPU wait like this,
        // but we can use waitUntilCompleted if needed, or MTLSharedEvent.
        if (fence) {
            [mtl_cmd_buffer waitUntilCompleted];
        }
    }
}

} // namespace Pathfinder
