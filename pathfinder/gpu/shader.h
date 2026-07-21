#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base.h"

namespace Pathfinder {

class Device;

enum class ShaderSourceType : uint8_t {
    SPIRV,
    GLSL,
    HLSL,
    MSL,
    GLSLES
};

enum class ShaderKind : uint8_t {
    VERTEX = 0,
    FRAGMENT,
    COMPUTE,
    GEOMETRY
};

/// ShaderSource Identifier.
struct ShaderCodeKey {
    ShaderSourceType source_type;
    uint8_t major_version;
    uint8_t minor_version;

    bool operator<(const ShaderCodeKey &rhs) const {
        uint32_t val = (static_cast<uint32_t>(source_type) << 16) | (static_cast<uint32_t>(major_version) << 8) |
                       static_cast<uint32_t>(minor_version);

        uint32_t rhs_val = (static_cast<uint32_t>(rhs.source_type) << 16) |
                           (static_cast<uint32_t>(rhs.major_version) << 8) | static_cast<uint32_t>(rhs.minor_version);

        return val < rhs_val;
    }
};

ShaderStage binary_shader_stage_to_shader_stage(ShaderKind kind);

// Pack the fields into a header struct for a single contiguous read operation
#pragma pack(push, 1)
struct ShdbinHeader {
    uint32_t magic;          // Magic number, e.g., 'SHDB'
    uint32_t format_version; // For future file format upgrades (e.g., version 1)
    ShaderSourceType source_type;
    uint32_t major_version = 0;
    uint32_t minor_version = 0;
    ShaderKind stage;
    uint32_t entry_point_size = 0;
    uint32_t code_size = 0;
};
#pragma pack(pop)

/// If you change this, you have to change how the shader builder reads shader files accordingly.
struct ShdbinInfo {
    ShdbinHeader header;
    std::vector<char> entry_point;
    std::vector<char> code;
};

// Predefined magic number for validation ('S' | 'H'<<8 | 'D'<<16 | 'B'<<24)
constexpr uint32_t SHDB_MAGIC = 0x42444853;

struct UniformBlockMember {
    std::string name;
    std::uint32_t offset;
    std::uint32_t size;
};

/// Complete reflection info for uniform blocks.
struct UniformBlockInfo {
    std::string name;
    std::uint32_t binding_point;
    std::uint32_t size;
    std::vector<UniformBlockMember> members;
};

/// Shader code for a specific backend.
struct ShaderCode {
    ShaderStage stage;
    std::string entry_point;
    std::string code;

    /// For GLES 3.0, in which we need to know the uniform names.
    std::vector<std::pair<uint32_t, std::string>> texture_binding_map;
    std::vector<std::pair<uint32_t, std::string>> uniform_buffer_binding_map;
    /// Currently unused.
    std::vector<UniformBlockInfo> ub_infos;
};

/// Raw shader sources for all backends.
class Shader final {
public:
    Shader() = default;

    ~Shader() = default;

    static std::shared_ptr<Shader> create_from_shdbin(const uint8_t *buffer, size_t length);

    void load_shdbin_from_bytes(const uint8_t *buffer, size_t length);

    void update_shader_code(const ShaderCodeKey &key, const std::shared_ptr<ShaderCode> &code);

    std::shared_ptr<ShaderCode> get_shader_code(const ShaderCodeKey &key);

private:
    std::map<ShaderCodeKey, std::shared_ptr<ShaderCode>> shader_codes_;
};

} // namespace Pathfinder
