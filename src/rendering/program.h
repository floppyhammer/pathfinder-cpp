//
// Created by floppyhammer on 8/26/2021.
//

#ifndef PATHFINDER_PROGRAM_H
#define PATHFINDER_PROGRAM_H

#include "../common/math/mat2x2.h"
#include "../common/math/mat4x4.h"
#include "../common/logger.h"
#include "../common/global_macros.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Pathfinder {
    class Program {
    public:
        /// Activate the shader.
        void use() const;

        /// Set sampler2D first, then bind texture to the corresponding binding point.
        void bind_texture(unsigned int binding_point,
                          const std::string &sampler_name,
                          unsigned int texture_id) const;

        void bind_uniform_buffer(unsigned int binding_point,
                                 const std::string &block_name,
                                 unsigned int ubo) const;

        // Utility uniform functions.
        // ------------------------------------------------------------------------
        void set_bool(const std::string &name, bool value) const;

        void set_int(const std::string &name, int value) const;

        void set_float(const std::string &name, float value) const;

        void set_vec2(const std::string &name, float x, float y) const;

        void set_vec2i(const std::string &name, int x, int y) const;

        void set_vec3(const std::string &name, float x, float y, float z) const;

        void set_vec4(const std::string &name, float x, float y, float z, float w) const;

        void set_mat2(const std::string &name, const Mat2x2<float> &mat) const;

        void set_mat4(const std::string &name, const Mat4x4<float> &mat) const;
        // ------------------------------------------------------------------------

    protected:
        /// Program ID.
        unsigned int id;

        /// Utility function for checking shader compilation/linking errors.
        static void check_compile_errors(GLuint shader, const std::string &type) {
            GLint success;
            GLchar info_log[1024];
            if (type != "PROGRAM") {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success) {
                    glGetShaderInfoLog(shader, 1024, nullptr, info_log);
                    std::ostringstream string_stream;
                    string_stream << "SHADER_COMPILATION_ERROR of type: " << type << "\n" << info_log;
                    Logger::error(string_stream.str(), "OpenGL");
                }
            } else {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success) {
                    glGetProgramInfoLog(shader, 1024, nullptr, info_log);
                    std::ostringstream string_stream;
                    string_stream << "PROGRAM_LINKING_ERROR of type: " << type << "\n" << info_log;
                    Logger::error(string_stream.str(), "OpenGL");
                }
            }
        }
    };
}

#endif //PATHFINDER_PROGRAM_H
