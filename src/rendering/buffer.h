//
// Created by floppyhammer on 4/9/2022.
//

#ifndef PATHFINDER_BUFFER_H
#define PATHFINDER_BUFFER_H

#include <cstdint>

namespace Pathfinder {
    enum class BufferType {
        Vertex,
        General,
    };

    enum class BufferUsage {
        Read,
        Write,
        ReadWrite,
    };

    class Buffer {
    public:
        BufferType type;
        BufferUsage usage;

        size_t offset;

        union Args {
            struct {
                uint32_t vao;
                uint32_t vbo;
            } vertex;
            struct {
                uint32_t sbo;
            } general;
        } args;
    };
}

#endif //PATHFINDER_BUFFER_H
