#ifndef PATHFINDER_GPU_FENCE_GL_H
#define PATHFINDER_GPU_FENCE_GL_H

#include "../fence.h"
#include "base.h"

namespace Pathfinder {

class FenceGl : public Fence {
    friend class DeviceGl;

public:
    ~FenceGl();

    void wait() const;

private:
    FenceGl() = default;

    GLsync fence{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_FENCE_GL_H
