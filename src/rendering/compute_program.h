//
// Created by floppyhammer on 6/25/2021.
//

#ifndef PATHFINDER_COMPUTE_PROGRAM_H
#define PATHFINDER_COMPUTE_PROGRAM_H

#include "program.h"
#include "../rendering/device_gl.h"
#include "../common/global_macros.h"

#ifdef PATHFINDER_USE_D3D11

namespace Pathfinder {
    class ComputeProgram : public Program {
    public:
        /// Constructor generates the shader on the fly.
        explicit ComputeProgram(const char *compute_path);

        explicit ComputeProgram(std::string compute_code);

        /// Bind a general buffer to a binding point.
        void bind_general_buffer(unsigned int binding_point, uint64_t buffer_id);

        void bind_image(unsigned int binding_point, unsigned int texture_id, int access_mode, int format) const;

        /// Launch computing.
        void dispatch(unsigned int group_size_x = 1, unsigned int group_size_y = 1, unsigned int group_size_z = 1);

    private:
        void compile(std::string& compute_code_s);
    };
}

#endif

#endif //PATHFINDER_COMPUTE_PROGRAM_H
