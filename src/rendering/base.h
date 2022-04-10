//
// Created by floppyhammer on 4/10/2022.
//

#ifndef PATHFINDER_BASE_H
#define PATHFINDER_BASE_H

#include "../common/global_macros.h"

namespace Pathfinder {
    enum class DataType {
        // Integers.
        BYTE = GL_BYTE, // 1 byte
        UNSIGNED_BYTE = GL_UNSIGNED_BYTE, // 1 byte
        SHORT = GL_SHORT, // 2 bytes
        UNSIGNED_SHORT = GL_UNSIGNED_SHORT, // 2 bytes
        INT = GL_INT, // 4 bytes
        UNSIGNED_INT = GL_UNSIGNED_INT, // 4 bytes

        // Floats.
        HALF_FLOAT = GL_HALF_FLOAT, // 2 bytes
    };
}

#endif //PATHFINDER_BASE_H
