#ifndef PATHFINDER_GPU_SHADER_MODULE_H
#define PATHFINDER_GPU_SHADER_MODULE_H

namespace Pathfinder {

class ShaderModule {
public:
    virtual ~ShaderModule() = default;

protected:
    ShaderModule() = default;

    std::string label_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_SHADER_MODULE_H
