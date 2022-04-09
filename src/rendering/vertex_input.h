//
// Created by floppyhammer on 4/9/2022.
//

#ifndef PATHFINDER_VERTEX_INPUT_H
#define PATHFINDER_VERTEX_INPUT_H

#include <cstdint>
#include <array>

namespace Pathfinder {
    enum class VertexInputRate {
        Vertex,
        Instance,
    };

    struct VertexInputBindingDescription {
        uint32_t binding;
        uint32_t stride;
        VertexInputRate input_rate;
    };

    struct VertexInputAttributeDescription {
        uint32_t binding;
        uint32_t location;
        uint32_t format;
        size_t offset;
    };

    struct VertexInputState {
        uint32_t binding_description_count;
        uint32_t attribute_description_count;
        VertexInputBindingDescription *binding_descriptions;
        VertexInputAttributeDescription *attribute_descriptions;
    };
}

#endif //PATHFINDER_VERTEX_INPUT_H
