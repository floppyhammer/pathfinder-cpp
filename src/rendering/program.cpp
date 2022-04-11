//
// Created by floppyhammer on 8/26/2021.
//

#include "program.h"

#include "device.h"

namespace Pathfinder {
    /// Activate the shader.
    void Program::use() const {
        glUseProgram(id);
    }

    unsigned int Program::get_id() const {
        return id;
    }

    // Utility uniform functions.
    // ------------------------------------------------------------------------
    void Program::set_bool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), (int) value);
    }

    void Program::set_int(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }

    void Program::set_float(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    }

    void Program::set_vec2(const std::string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
    }

    void Program::set_vec2i(const std::string &name, int x, int y) const {
        glUniform2i(glGetUniformLocation(id, name.c_str()), x, y);
    }

    void Program::set_vec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
    }

    void Program::set_vec4(const std::string &name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
    }

    void Program::set_mat2(const std::string &name, const Mat2x2<float> &mat) const {
        glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat.v[0]);
    }

    void Program::set_mat4(const std::string &name, const Mat4x4<float> &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat.v[0]);
    }
    // ------------------------------------------------------------------------
}
