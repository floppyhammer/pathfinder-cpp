//
// Created by floppyhammer on 6/25/2021.
//

#include "raster_program.h"

namespace Pathfinder {
    RasterProgram::RasterProgram(const std::string &vertex_code,
                                 const std::string &fragment_code) : Program() {
        compile(vertex_code.c_str(), fragment_code.c_str());
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
        id = glCreateProgram();
        glAttachShader(id, vertex);
        glAttachShader(id, fragment);
        glLinkProgram(id);
        check_compile_errors(id, "PROGRAM");

        // Delete the shaders as they're linked into our program now and no longer necessary.
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
}
