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

    void ComputeProgram::bind_general_buffer(unsigned int binding_point, uint64_t buffer_id) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, buffer_id);
    }

    void ComputeProgram::bind_image(unsigned int binding_point, unsigned int texture_id, int access_mode, int format) const {
        glBindImageTexture(binding_point, texture_id, 0, GL_FALSE, 0, access_mode, format);
    }

    void ComputeProgram::dispatch(unsigned int group_size_x, unsigned int group_size_y, unsigned int group_size_z) {
        if (group_size_x == 0 || group_size_y == 0 || group_size_z == 0) {
            Logger::error("Compute group size cannot be zero!", "ComputeProgram");
            return;
        }

        // Max global (total) work group counts x:2147483647 y:65535 z:65535.
        // Max local (in one shader) work group sizes x:1536 y:1024 z:64.
        glDispatchCompute(group_size_x, group_size_y, group_size_z);

        // In order to use timestamps more precisely.
#ifdef PATHFINDER_DEBUG
        glFinish();
#endif

        DeviceGl::check_error("ComputeProgram::dispatch");
    }
}

#endif
