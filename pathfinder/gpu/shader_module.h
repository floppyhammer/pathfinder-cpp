#pragma once

#include <string>

namespace Pathfinder {

class ShaderModule {
public:
    virtual ~ShaderModule() = default;

protected:
    ShaderModule() = default;

    std::string label_;
};

} // namespace Pathfinder
