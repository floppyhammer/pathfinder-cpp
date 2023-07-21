#ifndef PATHFINDER_GPU_COMPUTE_PIPELINE_H
#define PATHFINDER_GPU_COMPUTE_PIPELINE_H

#include <memory>
#include <string>

#include "data.h"

namespace Pathfinder {

class ComputePipeline {
public:
    virtual ~ComputePipeline() = default;

protected:
    std::string label;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_COMPUTE_PIPELINE_H
