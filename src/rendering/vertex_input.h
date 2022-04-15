#ifndef PATHFINDER_VERTEX_INPUT_H
#define PATHFINDER_VERTEX_INPUT_H

#include "gl/data.h"

#include <cstdint>
#include <array>

namespace Pathfinder {
    enum class VertexInputRate {
        VERTEX = 0,
        INSTANCE = 1,
    };

    struct VertexInputAttributeDescription {
        uint32_t binding;
        uint8_t size; // Must be 1, 2, 3, 4.
        DataType type;
        uint32_t stride;
        size_t offset;
        VertexInputRate vertex_input_rate;
    };
}

#endif //PATHFINDER_VERTEX_INPUT_H
