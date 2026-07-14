#pragma once

#include "../shader_module.h"
#include "device.h"

namespace Pathfinder {

class ShaderModuleMtl final : public ShaderModule {
    friend class DeviceMtl;

public:
    id<MTLFunction> function() const noexcept {
        return function_;
    }

private:
    id<MTLFunction> function_;
    id<MTLLibrary> library_;

    ShaderModuleMtl() = default;
};

} // namespace Pathfinder
