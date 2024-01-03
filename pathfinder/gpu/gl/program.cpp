#include "program.h"

#include <regex>

#include "../shader_module.h"
#include "shader_module.h"

namespace Pathfinder {

void Program::use() const {
    glUseProgram(id_);
}

unsigned int Program::get_handle() const {
    return id_;
}

void Program::set_bool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(id_, name.c_str()), (int)value);
}

void Program::set_int(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}

void Program::set_float(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}

void Program::set_vec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(id_, name.c_str()), x, y);
}

void Program::set_vec2i(const std::string &name, int x, int y) const {
    glUniform2i(glGetUniformLocation(id_, name.c_str()), x, y);
}

void Program::set_vec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(id_, name.c_str()), x, y, z);
}

void Program::set_vec4(const std::string &name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(id_, name.c_str()), x, y, z, w);
}

void Program::set_mat2(const std::string &name, const Mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &mat.v[0]);
}

void Program::set_mat4(const std::string &name, const Mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &mat.v[0]);
}

// RASTER PROGRAM

RasterProgram::RasterProgram(const std::shared_ptr<ShaderModule> &vertex_shader_module,
                             const std::shared_ptr<ShaderModule> &fragment_shader_module)
    : Program() {
    auto vertex_shader_module_gl = (ShaderModuleGl *)vertex_shader_module.get();
    auto fragment_shader_module_gl = (ShaderModuleGl *)fragment_shader_module.get();

    // Set up shader program.
    id_ = glCreateProgram();
    glAttachShader(id_, vertex_shader_module_gl->get_handle());
    glAttachShader(id_, fragment_shader_module_gl->get_handle());
    glLinkProgram(id_);
    check_compile_errors();
}

// COMPUTE PROGRAM

ComputeProgram::ComputeProgram(const std::shared_ptr<ShaderModule> &compute_shader_module) : Program() {
    auto compute_shader_module_gl = (ShaderModuleGl *)compute_shader_module.get();

    // Shader program.
    id_ = glCreateProgram();
    glAttachShader(id_, compute_shader_module_gl->get_handle());
    glLinkProgram(id_);
    check_compile_errors();
}

} // namespace Pathfinder
