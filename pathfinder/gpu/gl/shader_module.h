#ifndef PATHFINDER_GPU_SHADER_MODULE_GL_H
#define PATHFINDER_GPU_SHADER_MODULE_GL_H

#include <vector>
#include <cstdint>

#include "../shader_module.h"

namespace Pathfinder {

enum class ShaderStage;

class ShaderModuleGl : public ShaderModule {
    friend class DeviceGl;
    friend class RasterProgram;
    friend class ComputeProgram;

public:
    ~ShaderModuleGl() override;

    unsigned int get_handle() const;

private:
    ShaderModuleGl(const std::vector<char>& source_code,
                   ShaderStage shader_stage,
                   const std::vector<std::pair<uint32_t, std::string>>& texture_binding_map,
                   const std::vector<std::pair<uint32_t, std::string>>& uniform_buffer_binding_map,
                   const std::string& label = "");

    /// Utility function for checking shader compilation errors.
    void check_compile_errors() const;

    unsigned int id_{};

    std::vector<std::pair<uint32_t, std::string>> texture_binding_map_;
    std::vector<std::pair<uint32_t, std::string>> uniform_buffer_binding_map_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_SHADER_MODULE_GL_H
