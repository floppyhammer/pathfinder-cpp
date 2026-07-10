#include <CLI/CLI.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "../../pathfinder/common/math/basic.h"
#include "../../pathfinder/gpu/shader.h"
#include "generator.h"

namespace Pathfinder {
enum class ShaderKind : uint8_t;
}
using namespace Pathfinder;

// For debug: -i texture_quad.vert -o texture_quad_vert.shdbin -t vert

int main(int argc, char* argv[]) {
    CLI::App app{"Shader generator"};

    std::string input_file;
    app.add_option("-i,--input", input_file, "Input shader file")->required();

    std::string output_file;
    app.add_option("-o,--output", output_file, "Output shader file")->required();

    std::string output_header;
    app.add_option("-u,--output_header", output_header, "Output C++ header file, for embedding shaders in source code");

    auto shader_type_validator = CLI::IsMember({"vert", "frag", "comp"});
    std::string shader_type_str;
    app.add_option("-t,--type", shader_type_str, "Shader type: vert, frag, comp")
        ->required()
        ->check(shader_type_validator);

    int es_version = 300;
    auto es_version_validator = CLI::IsMember({300, 310, 320});
    app.add_option("-e,--es_version", es_version, "Generated gles version: 300, 310, 320")->check(es_version_validator);

    bool print_flag = false;
    app.add_flag("-p,--print", print_flag, "Print output shaders");

    CLI11_PARSE(app, argc, argv)

    std::map<std::string, ShaderKind> shader_type_map;
    shader_type_map["vert"] = ShaderKind::VERTEX;
    shader_type_map["frag"] = ShaderKind::FRAGMENT;
    shader_type_map["comp"] = ShaderKind::COMPUTE;

    auto pos = std::max(output_header.find_last_of('\\'), output_header.find_last_of('/'));

    auto start_pos = pos == std::string::npos ? 0 : pos;
    std::string var_name = output_header.substr(start_pos, output_header.size() - 2 - start_pos);

    generate_shader(input_file.c_str(), output_file.c_str(), shader_type_map[shader_type_str]);

    std::cout << "Generated shader file successfully: " << output_file << std::endl;

    return 0;
}
