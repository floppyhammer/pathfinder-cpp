#pragma once

#include <cstdint>

#include "../texture.h"

namespace Pathfinder {

class SamplerGl : public Sampler {
public:
    explicit SamplerGl(SamplerDescriptor descriptor);

    ~SamplerGl() override;

    uint32_t get_handle() const {
        return id_;
    }

private:
    uint32_t id_{};
};

} // namespace Pathfinder
