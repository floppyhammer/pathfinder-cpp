#pragma once

#include <string>

namespace Pathfinder {

class SwapChain;

class Fence {
public:
    Fence() = default;

    std::string label;
};

} // namespace Pathfinder
