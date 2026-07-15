#include "queue.h"

#include "command_encoder.h"

namespace Pathfinder {

void QueueMtl::submit(const std::shared_ptr<CommandEncoder>& encoder, const std::shared_ptr<Fence>& fence) {
    auto encoder_mtl = (CommandEncoderMtl*)encoder.get();

    @autoreleasepool {
        encoder_mtl->prepare();

        auto mtl_cmd_buffer = encoder_mtl->get_handle();

        [mtl_cmd_buffer commit];

        // If fence is provided, we should probably handle it.
        // Metal doesn't have a direct equivalent to Vulkan's fence for CPU wait like this,
        // but we can use waitUntilCompleted if needed, or MTLSharedEvent.
        if (fence) {
            [mtl_cmd_buffer waitUntilCompleted];
        }

        encoder_mtl->clear_pending_ops();
    }
}

} // namespace Pathfinder
