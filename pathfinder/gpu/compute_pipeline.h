#ifndef PATHFINDER_GPU_COMPUTE_PIPELINE_H
#define PATHFINDER_GPU_COMPUTE_PIPELINE_H

#include <string>

namespace Pathfinder {

class ComputePipeline {
public:
    virtual ~ComputePipeline() = default;

protected:
    std::string label_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_COMPUTE_PIPELINE_H
