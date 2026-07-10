#include "shader.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace Pathfinder {

struct membuf : std::streambuf {
    membuf(const uint8_t *begin, const uint8_t *end) {
        this->setg(reinterpret_cast<char *>(const_cast<uint8_t *>(begin)),
                   reinterpret_cast<char *>(const_cast<uint8_t *>(begin)),
                   reinterpret_cast<char *>(const_cast<uint8_t *>(end)));
    }
};

ShaderStage binary_shader_stage_to_shader_stage(ShaderKind kind) {
    switch (kind) {
        case ShaderKind::VERTEX: {
            return ShaderStage::Vertex;
        }
        case ShaderKind::FRAGMENT: {
            return ShaderStage::Fragment;
        }
        case ShaderKind::COMPUTE: {
            return ShaderStage::Compute;
        }
        case ShaderKind::GEOMETRY: {
            return ShaderStage::Geometry;
        }
        default:
            abort();
    }
}

bool read_shdbin(std::istream &in, ShdbinInfo &shader) {
    // 1. Read the entire header at once for better performance
    if (!in.read(reinterpret_cast<char *>(&shader.header), sizeof(ShdbinHeader))) {
        return false;
    }

    // 2. Validate file integrity via the magic number
    if (shader.header.magic != SHDB_MAGIC) {
        // Optional: Add logging here (e.g., "Invalid file format or not a valid shdbin file")
        return false;
    }

    // 3. Read and process the Entry Point (safely appending the null terminator '\0')
    shader.entry_point.resize(shader.header.entry_point_size + 1);
    if (!in.read(shader.entry_point.data(), shader.header.entry_point_size)) {
        return false;
    }
    shader.entry_point.back() = '\0';

    // 4. Read the Shader Code
    if (shader.header.source_type == ShaderSourceType::SPIRV) {
        shader.code.resize(shader.header.code_size);
    } else {
        // For text-based shaders (GLSL/MSL), allocate an extra byte for the null terminator
        shader.code.resize(shader.header.code_size + 1);
        shader.code.back() = '\0';
    }

    return (bool)in.read(shader.code.data(), shader.header.code_size);
}

void convert_shdbin_info_to_shader_code(const ShdbinInfo &shader_info, ShaderCodeKey &shader_key, ShaderCode &shader_code) {
    const auto &shader_header = shader_info.header;
    shader_key.major_version = shader_header.major_version;
    shader_key.minor_version = shader_header.minor_version;
    shader_key.source_type = shader_header.source_type;

    shader_code.stage = binary_shader_stage_to_shader_stage(shader_info.header.stage);
    shader_code.code = std::string(shader_info.code.begin(), shader_info.code.end());
    shader_code.entry_point = std::string(shader_info.entry_point.begin(), shader_info.entry_point.end());

    if (shader_header.source_type == ShaderSourceType::GLSL || shader_header.source_type == ShaderSourceType::GLSLES) {
        std::string &code = shader_code.code;
        const std::string start_marker = "//BINDING_START";
        const std::string end_marker = "//BINDING_END";

        size_t start_pos = code.find(start_marker);
        size_t end_pos = code.find(end_marker);

        if (start_pos != std::string::npos && end_pos != std::string::npos && end_pos > start_pos) {
            // 1. Extract and parse metadata
            size_t meta_start = start_pos + start_marker.length();
            std::string metadata = code.substr(meta_start, end_pos - meta_start);

            std::istringstream iss(metadata);
            std::string line;
            while (std::getline(iss, line)) {
                if (line.compare(0, 6, "//TEX:") == 0) {
                    size_t last_colon = line.find_last_of(':');
                    if (last_colon != std::string::npos) {
                        std::string name = line.substr(6, last_colon - 6);
                        uint32_t binding = std::stoi(line.substr(last_colon + 1));
                        shader_code.texture_binding_map.emplace_back(binding, name);
                    }
                } else if (line.compare(0, 6, "//UBO:") == 0) {
                    size_t last_colon = line.find_last_of(':');
                    if (last_colon != std::string::npos) {
                        std::string name = line.substr(6, last_colon - 6);
                        uint32_t binding = std::stoi(line.substr(last_colon + 1));
                        shader_code.uniform_buffer_binding_map.emplace_back(binding, name);
                    }
                }
            }

            // 2. Erase metadata block from source code
            size_t total_end = end_pos + end_marker.length();
            code.erase(start_pos, total_end - start_pos);

            // 3. Trim leading whitespace/newlines to ensure #version is at the top
            size_t first_char = code.find_first_not_of(" \t\r\n");
            if (first_char != std::string::npos) {
                code.erase(0, first_char);
            }
        }
    }
}

std::shared_ptr<Shader> Shader::create_from_shdbin(const uint8_t *buffer, const size_t length) {
    auto shader = std::make_shared<Shader>();
    shader->load_shdbin_from_bytes(buffer, length);
    return shader;
}

void Shader::load_shdbin_from_bytes(const uint8_t *buffer, const size_t length) {
    membuf sbuf(buffer, buffer + length);
    std::istream in(&sbuf);

    ShaderCodeKey key{};

    while (!in.eof()) {
        ShdbinInfo shader_info;
        if (!read_shdbin(in, shader_info)) {
            break;
        }
        auto code = std::make_shared<ShaderCode>();
        convert_shdbin_info_to_shader_code(shader_info, key, *code);
        update_shader_code(key, code);
    }
}

void Shader::update_shader_code(const ShaderCodeKey &key, const std::shared_ptr<ShaderCode> &code) {
    shader_codes_[key] = code;
}

std::shared_ptr<ShaderCode> Shader::get_shader_code(const ShaderCodeKey &key) {
    const auto itr = shader_codes_.find(key);
    if (itr == shader_codes_.end()) {
        return nullptr;
    }
    return itr->second;
}

} // namespace Pathfinder
