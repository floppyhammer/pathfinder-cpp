#ifndef PATHFINDER_GPU_FENCE_H
#define PATHFINDER_GPU_FENCE_H

#include <string>

namespace Pathfinder {

class SwapChain;

class Fence {
public:
    Fence() = default;

    std::string label;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_FENCE_H
