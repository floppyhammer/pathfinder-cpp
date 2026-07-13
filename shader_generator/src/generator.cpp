#include "generator.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <spirv_parser.hpp>

#include "../../pathfinder/gpu/shader_translator.h"

using namespace Pathfinder;

bool write_shdbin(std::ostream& out, ShdbinInfo& shader) {
    // 1. Auto-fill crucial header information to prevent human error
    shader.header.magic = SHDB_MAGIC;
    shader.header.format_version = 1; // Current file format version

    // 2. Calculate the exact payload size for the Entry Point.
    // If the vector already contains a null terminator '\0', we strip it from the disk size
    // because read_shdbin() will automatically append it back.
    uint32_t ep_size = static_cast<uint32_t>(shader.entry_point.size());
    if (ep_size > 0 && shader.entry_point.back() == '\0') {
        ep_size -= 1;
    }
    shader.header.entry_point_size = ep_size;

    // 3. Calculate the exact payload size for the Shader Code.
    uint32_t c_size = static_cast<uint32_t>(shader.code.size());
    // Only text-based shaders might have trailing null terminators
    if (shader.header.source_type != ShaderSourceType::SPIRV) {
        if (c_size > 0 && shader.code.back() == '\0') {
            c_size -= 1;
        }
    }
    shader.header.code_size = c_size;

    // 4. Write the entire Header as a single contiguous block
    if (!out.write(reinterpret_cast<const char*>(&shader.header), sizeof(ShdbinHeader))) {
        return false;
    }

    // 5. Write the Entry Point string (excluding the trailing '\0' if we adjusted the size)
    if (ep_size > 0) {
        if (!out.write(shader.entry_point.data(), ep_size)) {
            return false;
        }
    }

    // 6. Write the Shader Code payload (excluding the trailing '\0' for text shaders)
    if (c_size > 0) {
        if (!out.write(shader.code.data(), c_size)) {
            return false;
        }
    }

    // Flush the stream to ensure all data is written to disk immediately
    out.flush();
    return out.good();
}

void generate_shader(const char* input_shader_file, const char* output_binary_file, ShaderKind stage) {
    // 1. 使用 std::ifstream 读取输入文件，更符合 C++ 习惯且安全
    std::ifstream ifs(input_shader_file, std::ios::binary | std::ios::ate);
    if (!ifs.is_open()) {
        std::cerr << "Input file does not exist or cannot be opened: " << input_shader_file << std::endl;
        return;
    }

    std::streamsize length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::string shader_source(static_cast<size_t>(length), '\0');
    if (!ifs.read(&shader_source[0], length)) {
        std::cerr << "Failed to read input file: " << input_shader_file << std::endl;
        return;
    }
    ifs.close();

    // 2. 使用 std::ofstream 以二进制模式打开输出文件
    std::ofstream ofs(output_binary_file, std::ios::binary);
    if (!ofs.is_open()) {
        std::cerr << "Failed to open output file: " << output_binary_file << std::endl;
        return;
    }

    // Handle subpassLoad.
    bool need_framebuffer_fetch = (shader_source.find("subpassLoad") != std::string::npos);

    // TODO: 可以考虑从命令行参数传入 entry_point
    std::string entry_point = "main";

    // 3. 使用 ShaderTranslator 进行编译和转换
    auto translator = std::make_shared<ShaderTranslator>(binary_shader_stage_to_shader_stage(stage));
    translator->compile_from_glsl(entry_point, shader_source, need_framebuffer_fetch);

    auto prepared_shader = translator->get_shader();

    // 辅助 lambda 函数，用于写入不同类型的 Shader 到同一个二进制文件中
    auto write_to_bin = [&](ShaderSourceType type, uint8_t major, uint8_t minor) {
        auto shader_code = prepared_shader->get_shader_code(ShaderCodeKey{type, major, minor});
        if (!shader_code) {
            return;
        }

        ShdbinInfo bin_info;
        bin_info.header.source_type = type;
        bin_info.header.major_version = major;
        bin_info.header.minor_version = minor;
        bin_info.header.stage = stage;

        bin_info.entry_point = std::vector<char>(entry_point.begin(), entry_point.end());
        bin_info.code = std::vector<char>(shader_code->code.begin(), shader_code->code.end());

        if (!write_shdbin(ofs, bin_info)) {
            std::cerr << "Failed to write shader data for type: " << (int)type << std::endl;
        }
    };

    // 4. 依次写入各个后端的 Shader 代码
    // SPIRV
    write_to_bin(ShaderSourceType::SPIRV, 1, 1);

    // GLSL (使用转换后的代码，包含 Binding 元数据)
    write_to_bin(ShaderSourceType::GLSL, 4, 5);

    // GLSLES
    uint8_t gles_minor = (stage == ShaderKind::COMPUTE ? 1 : 0);
    write_to_bin(ShaderSourceType::GLSLES, 3, gles_minor);

    // MSL
    write_to_bin(ShaderSourceType::MSL, 1, 2);

    ofs.close();
}
