#include "program.h"

#include <regex>

namespace Pathfinder {

void Program::use() const {
    glUseProgram(id_);
}

unsigned int Program::get_id() const {
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

RasterProgram::RasterProgram(const std::vector<char> &vertex_code, const std::vector<char> &fragment_code) : Program() {
    /// Has to pass string.c_str(), as vector<char>.data() doesn't work.
    std::string vert_string = {vertex_code.begin(), vertex_code.end()};
    std::string frag_string = {fragment_code.begin(), fragment_code.end()};

#ifdef PATHFINDER_MINIMUM_SHADER_VERSION_SUPPORT
    vert_string = std::regex_replace(vert_string, std::regex("#version 310 es"), "#version 300 es");
    frag_string = std::regex_replace(frag_string, std::regex("#version 310 es"), "#version 300 es");
#endif

    compile(vert_string.c_str(), frag_string.c_str());
}

void RasterProgram::compile(const char *vertex_code, const char *fragment_code) {
    // Vertex shader.
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_code, nullptr);
    glCompileShader(vertex);
    check_compile_errors(vertex, "VERTEX");

    // Fragment Shader.
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_code, nullptr);
    glCompileShader(fragment);
    check_compile_errors(fragment, "FRAGMENT");

    // Set up shader program.
    id_ = glCreateProgram();
    glAttachShader(id_, vertex);
    glAttachShader(id_, fragment);
    glLinkProgram(id_);
    check_compile_errors(id_, "PROGRAM");

    // Delete the shaders as they're linked into our program now and no longer necessary.
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

// COMPUTE PROGRAM

ComputeProgram::ComputeProgram(const std::vector<char> &compute_code) : Program() {
    /// Has to pass string.c_str(), as vector<char>.data() doesn't work.
    std::string compute_string = {compute_code.begin(), compute_code.end()};

#ifdef PATHFINDER_MINIMUM_SHADER_VERSION_SUPPORT
    compute_string = std::regex_replace(compute_string, std::regex("#version 430"), "#version 310 es");
#endif

    compile(compute_string.c_str());
}

void ComputeProgram::compile(const char *compute_code) {
#ifdef PATHFINDER_ENABLE_D3D11
    // Compile shaders.
    unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &compute_code, nullptr);
    glCompileShader(compute);
    check_compile_errors(compute, "COMPUTE");

    // Shader program.
    id_ = glCreateProgram();
    glAttachShader(id_, compute);
    glLinkProgram(id_);
    check_compile_errors(id_, "PROGRAM");

    // Delete the shaders as they're linked into our program now and no longer necessary.
    glDeleteShader(compute);
#endif
}

} // namespace Pathfinder
