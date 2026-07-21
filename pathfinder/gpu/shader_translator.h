#pragma once

#include <memory>
#include <string>
#include <vector>

#include "base.h"
#include "shader.h"

namespace Pathfinder {

/// Translates GLSL/SPV into SPV, MSL, GLSL ES etc. at runtime. Optional.
class ShaderTranslator {
public:
    explicit ShaderTranslator(ShaderStage stage);

    ~ShaderTranslator();

    /// Set shader content by a specific key, shaders of other keys won't be affected.
    void set_shader_code(const ShaderCodeKey &key, const std::shared_ptr<ShaderCode> &code);

    /// Compile shader module at runtime using provided Glsl 4.4 code.
    void compile_from_glsl(const std::string &entry_point, const std::string &shader_code, bool use_framebuffer_fetch);

    std::shared_ptr<Shader> get_shader() const;

private:
    bool prepare(bool use_framebuffer_fetch);

    std::string name_;
    ShaderStage stage_;
    std::shared_ptr<Shader> shader_;
};

} // namespace Pathfinder
