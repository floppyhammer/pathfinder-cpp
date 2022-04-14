//
// Created by floppyhammer on 8/26/2021.
//

#ifndef PATHFINDER_DEVICE_H
#define PATHFINDER_DEVICE_H

#include "buffer.h"
#include "texture.h"
#include "command_buffer.h"
#include "../common/math/basic.h"
#include "../common/global_macros.h"
#include "../common/logger.h"

#include <vector>

namespace Pathfinder {
    class Device {
    public:
        Device() = default;

        ~Device() = default;

        static std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size);

        static std::shared_ptr<Texture> create_texture(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type);

        static std::shared_ptr<CommandBuffer> create_command_buffer();
    };
}

#endif //PATHFINDER_DEVICE_H
