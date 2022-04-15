//
// Created by floppyhammer on 7/5/2021.
//

#include "compute_program.h"

#include "../common/io.h"
#include "../common/logger.h"

#ifdef PATHFINDER_USE_D3D11

namespace Pathfinder {
    ComputeProgram::ComputeProgram(const std::string &compute_code) : Program() {
        compile(compute_code.c_str());
    }

    void ComputeProgram::compile(const char *compute_code) {
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
