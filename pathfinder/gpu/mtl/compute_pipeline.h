#pragma once

#include <MetalKit/MetalKit.h>

#include "../compute_pipeline.h"
#include "../device.h"

namespace Pathfinder {

class ComputePipelineMtl final : public ComputePipeline {
    friend class DeviceMtl;

public:
    id<MTLComputePipelineState> get_handle() {
        return pipeline_state_;
    }

private:
    ComputePipelineMtl() {}

    id<MTLComputePipelineState> pipeline_state_ = nil;
};

} // namespace Pathfinder
