//
// Created by floppyhammer on 6/25/2021.
//

#ifndef PATHFINDER_RASTER_PROGRAM_H
#define PATHFINDER_RASTER_PROGRAM_H

#include "program.h"
#include "../../common/global_macros.h"
#include "../../common/io.h"

namespace Pathfinder {
    class RasterProgram : public Program {
    public:
        /// Has to use string, as vector<char> won't work.
        RasterProgram(const std::string &vertex_code,
                      const std::string &fragment_code);

    private:
        void compile(const char *vertex_code, const char *fragment_code);
    };
}

#endif //PATHFINDER_RASTER_PROGRAM_H
