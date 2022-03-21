//
// Created by floppyhammer on 6/25/2021.
//

#ifndef PATHFINDER_RASTER_PROGRAM_H
#define PATHFINDER_RASTER_PROGRAM_H

#include "program.h"
#include "../common/global_macros.h"
#include "../common/io.h"

namespace Pathfinder {
    class RasterProgram : public Program {
    public:
        /// Constructor generates the shader on the fly.
        RasterProgram(std::string vertex_code, std::string fragment_code);

        RasterProgram(const char *vertex_path, const char *fragment_path);

    private:
        void compile(std::string &vertex_code_s, std::string &fragment_code_s);
    };
}

#endif //PATHFINDER_RASTER_PROGRAM_H
