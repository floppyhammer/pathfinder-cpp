#include "fence.h"

#include "device.h"

namespace Pathfinder {

FenceGl::~FenceGl() {
    glDeleteSync(fence);
}

void FenceGl::wait() const {
    // Wait indefinitely
    while (true) {
        GLenum wait_result = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);

        if (wait_result == GL_ALREADY_SIGNALED || wait_result == GL_CONDITION_SATISFIED) {
            return;
        }
    }
}

} // namespace Pathfinder
