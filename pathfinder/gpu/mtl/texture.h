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

private:
    TextureMtl(const TextureDescriptor &texture_descriptor, const std::string &label);

    id<MTLTexture> mtl_texture_ = nil;
};

} // namespace Pathfinder
