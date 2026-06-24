#ifndef PATHFINDER_GPU_COMPUTE_PIPELINE_H
#define PATHFINDER_GPU_COMPUTE_PIPELINE_H

#include <string>

#include "descriptor_set.h"

namespace Pathfinder {

class ComputePipeline {
public:
    virtual ~ComputePipeline() = default;

protected:
    std::shared_ptr<DescriptorSetLayout> layout_;

    std::string label_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_COMPUTE_PIPELINE_H
