#pragma once

#include <MetalKit/MetalKit.h>

#include "../texture.h"

namespace Pathfinder {

class TextureMtl final : public Texture {
    friend class DeviceMtl;
    friend class CommandEncoderMtl;

public:
    id<MTLTexture> get_handle() {
        return mtl_texture_;
    }

    id<MTLBuffer> get_staging_buffer(id<MTLDevice> device) {
        if (!staging_buffer_) {
            // Bytes of one pixel.
            auto pixel_size = get_pixel_size(get_format());

            uint32_t max_data_size = get_size().area() * pixel_size;

            staging_buffer_ = [device newBufferWithLength:max_data_size options:MTLResourceStorageModeShared];
        }
        return staging_buffer_;
    }

private:
    TextureMtl(const TextureDescriptor &texture_descriptor, const std::string &label);

    id<MTLTexture> mtl_texture_ = nil;
    id<MTLBuffer> staging_buffer_ = nil;
};

} // namespace Pathfinder
