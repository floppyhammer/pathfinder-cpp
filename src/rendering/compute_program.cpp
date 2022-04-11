//
// Created by floppyhammer on 7/5/2021.
//

#include "compute_program.h"

#include "../common/io.h"
#include "../common/logger.h"

#ifdef PATHFINDER_USE_D3D11

namespace Pathfinder {
    ComputeProgram::ComputeProgram(std::string compute_code) : Program() {
        compile(compute_code);
    }

    ComputeProgram::ComputeProgram(const char *computePath) : Program() {
        // Retrieve source code from file.
        std::string compute_code = load_file_as_string(computePath);

        compile(compute_code);
    }

    void ComputeProgram::compile(std::string& compute_code_s) {
        // Convert std::strings to C strings.
        const char *compute_code = compute_code_s.c_str();

        // Compile shaders.
        unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &compute_code, nullptr);
        glCompileShader(compute);
        check_compile_errors(compute, "COMPUTE");

        // Shader program.
        id = glCreateProgram();
        glAttachShader(id, compute);
        glLinkProgram(id);
        check_compile_errors(id, "PROGRAM");

        // Delete the shaders as they're linked into our program now and no longer necessary.
        glDeleteShader(compute);
    }
}

#endif
