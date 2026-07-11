#pragma once

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
