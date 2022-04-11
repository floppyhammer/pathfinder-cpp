//
// Created by floppyhammer on 4/9/2022.
//

#ifndef PATHFINDER_BUFFER_H
#define PATHFINDER_BUFFER_H

#include "../common/global_macros.h"

#include <cstdint>

namespace Pathfinder {
    enum class BufferType {
        Vertex,
        Uniform,
        General,
    };

    enum class GeneralBufferUsage {
        Read,
        Write,
        ReadWrite,
    };

    class Buffer {
    public:
        ~Buffer() {
            unsigned int buffer_id;

            switch (type) {
                case BufferType::Vertex: {
                    buffer_id = args.vertex.vbo;
                }
                case BufferType::Uniform: {
                    buffer_id = args.uniform.ubo;
                }
                    break;
                case BufferType::General: {
                    buffer_id = args.general.sbo;
                }
                    break;
            }

            glDeleteBuffers(1, &buffer_id);
        };

        BufferType type;
        size_t size;

        union Args {
            struct {
                uint32_t vbo;
            } vertex;
            struct {
                uint32_t ubo;
            } uniform;
            struct {
                uint32_t sbo;
            } general;
        } args;
    };
}

#endif //PATHFINDER_BUFFER_H
