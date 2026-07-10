#pragma once

#include <memory>
#include <string>
#include <vector>

#include "base.h"
#include "shader.h"

namespace Pathfinder {

/// For GLES 3.0 compatibility
struct UniformBufferElement {
    std::string name;
    std::uint32_t offset;
    std::uint32_t size;
};

struct UniformBufferInfo {
    std::string name;
    std::uint32_t binding_point;
    std::uint32_t size;
    ShaderStage stage;
    std::vector<UniformBufferElement> elements;
};

/// Translates GLSL/SPV into SPV, MSL, GLSL ES etc. at runtime. Optional.
class ShaderTranslator {
public:
    ShaderTranslator(ShaderStage stage);

    ~ShaderTranslator();

    /// Set shader content by a specific key, shaders of other keys won't be affected.
    void set_shader(const ShaderCodeKey &key, const std::shared_ptr<ShaderCode> &code);

    /// Compile shader module at runtime using provided Glsl 4.4 code.
    void compile_from_glsl(const std::string &entry_point, const std::string &shader_code, bool need_framebuffer_fetch);

    std::shared_ptr<Shader> outputShader() const {
        return shader_;
    }

private:
    bool prepare(bool need_framebuffer_fetch);

    std::string name_;
    ShaderStage stage_;
    std::shared_ptr<Shader> shader_;
    std::vector<UniformBufferInfo> ubo_infos_;
};

} // namespace Pathfinder
