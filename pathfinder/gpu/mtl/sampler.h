#pragma once

#include "../texture.h"

namespace Pathfinder {

class SamplerMtl final : public Sampler {
    friend class DeviceMtl;

public:
    id<MTLSamplerState> get_handle() noexcept {
        return mtl_sampler_;
    }

private:
    SamplerMtl(const SamplerDescriptor &descriptor) : Sampler(descriptor) {}

    id<MTLSamplerState> mtl_sampler_ = nil;
};

} // namespace Pathfinder
