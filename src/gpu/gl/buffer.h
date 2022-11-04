#ifndef PATHFINDER_GPU_BUFFER_GL_H
#define PATHFINDER_GPU_BUFFER_GL_H

#include <cstdint>

#include "../../common/global_macros.h"
#include "../buffer.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class BufferGl : public Buffer {
public:
    BufferGl(BufferType _type, size_t _size, MemoryProperty _memory_property);

    ~BufferGl();

    uint32_t id;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_BUFFER_GL_H
