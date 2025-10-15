#include "fence.h"

#include "device.h"

namespace Pathfinder {

FenceGl::~FenceGl() {
    glDeleteSync(fence);
}

void FenceGl::wait() const {
    GLenum wait_result = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    assert(wait_result == GL_ALREADY_SIGNALED || wait_result == GL_CONDITION_SATISFIED);
}

} // namespace Pathfinder
