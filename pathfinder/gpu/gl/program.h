#ifndef PATHFINDER_GPU_PROGRAM_H
#define PATHFINDER_GPU_PROGRAM_H

#include <fstream>
#include <sstream>
#include <string>

#include "../../common/logger.h"
#include "../../common/math/mat2.h"
#include "../../common/math/mat4.h"
#include "base.h"

namespace Pathfinder {

class ShaderModule;

class Program {
public:
    /// Activate the shader.
    void use() const;

    unsigned int get_handle() const;

    // Utility uniform functions.
    // ------------------------------------------------------
    void set_bool(const std::string &name, bool value) const;

    void set_int(const std::string &name, int value) const;

    void set_float(const std::string &name, float value) const;

    void set_vec2(const std::string &name, float x, float y) const;

    void set_vec2i(const std::string &name, int x, int y) const;

    void set_vec3(const std::string &name, float x, float y, float z) const;

    void set_vec4(const std::string &name, float x, float y, float z, float w) const;

    void set_mat2(const std::string &name, const Mat2 &mat) const;

    void set_mat4(const std::string &name, const Mat4 &mat) const;
    // ------------------------------------------------------

protected:
    /// Program ID.
    unsigned int id_{};

    /// Utility function for checking shader linking errors.
    void check_compile_errors() const {
        GLint success;
        glGetProgramiv(id_, GL_LINK_STATUS, &success);

        if (!success) {
            GLchar info_log[1024];
            glGetProgramInfoLog(id_, 1024, nullptr, info_log);

            std::ostringstream string_stream;
            string_stream << "PROGRAM_LINKING_ERROR:"
                          << "\n"
                          << info_log;
            Logger::error(string_stream.str());
        }
    }
};

class RasterProgram : public Program {
public:
    RasterProgram(const std::shared_ptr<ShaderModule> &vertex_shader_module,
                  const std::shared_ptr<ShaderModule> &fragment_shader_module);
};

class ComputeProgram : public Program {
public:
    explicit ComputeProgram(const std::shared_ptr<ShaderModule> &compute_shader_module);
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_PROGRAM_H
