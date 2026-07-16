#include "queue.h"

#include "command_encoder.h"

namespace Pathfinder {

void QueueMtl::submit(const std::shared_ptr<CommandEncoder>& encoder, const std::shared_ptr<Fence>& fence) {
    auto encoder_mtl = (CommandEncoderMtl*)encoder.get();

    @autoreleasepool {
        encoder_mtl->prepare();

        auto mtl_cmd_buffer = encoder_mtl->get_handle();

        // We must create a local shared_ptr copy to ensure the block captures the object by value,
        // rather than capturing the reference to the parameter which will go out of scope.
        auto encoder_captured = encoder;

        [mtl_cmd_buffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
            encoder_captured->invoke_callbacks();
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
