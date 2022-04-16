//
// Created by floppyhammer on 4/9/2022.
//

#ifndef PATHFINDER_BUFFER_GL_H
#define PATHFINDER_BUFFER_GL_H

#include "../buffer.h"
#include "../../common/global_macros.h"

#include <cstdint>

namespace Pathfinder {
    class BufferGl : public Buffer {
    public:
        BufferGl(BufferType p_type, size_t p_size);

        ~BufferGl() {
            glDeleteBuffers(1, &id);
        };

        uint32_t id;
    };
}

#endif //PATHFINDER_BUFFER_GL_H
