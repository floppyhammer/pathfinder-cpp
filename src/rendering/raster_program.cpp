//
// Created by chy on 6/25/2021.
//

#include "raster_program.h"

namespace Pathfinder {
    RasterProgram::RasterProgram(std::string vertex_code, std::string fragment_code) : Program() {
        compile(vertex_code, fragment_code);
    }

    RasterProgram::RasterProgram(const char *vertex_path, const char *fragment_path) : Program() {
        // Retrieve the vertex/fragment source code from file.
        std::string vertex_code = load_file_as_string(vertex_path);
        std::string fragment_code = load_file_as_string(fragment_path);

        compile(vertex_code, fragment_code);
    }

    void RasterProgram::compile(std::string &vertex_code_s, std::string &fragment_code_s) {
        // Convert std::strings to C strings.
        const char *vertex_code = vertex_code_s.c_str();
        const char *fragment_code = fragment_code_s.c_str();

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
