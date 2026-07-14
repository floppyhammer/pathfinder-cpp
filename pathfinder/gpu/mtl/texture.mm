#include "texture.h"

#include <cstdint>
#include <iostream>

#include "base.h"
#include "buffer.h"
#include "command_encoder.h"

namespace Pathfinder {

TextureMtl::TextureMtl(const TextureDescriptor &texture_descriptor, const std::string &label)
    : Texture(texture_descriptor) {}
//
// TextureView *TextureMtl::get_view(const TextureViewDescriptor &texture_view_desc) {
//    auto ref_tex = texture->getMtlTexture();
//    texture_ = texture;
//    if (texture->getDefaultTextureViewDescriptor() == desc) {
//        view_texture_ = texture->getMtlTexture();
//    } else {
//        auto pixel_format = to_mtl_pixel_format(texture->getFormat());
//        auto texture_type = to_mtl_texture_type(texture->getType());
//        auto levels = texture->getLevel();
//        // fixme: use identity swizzle
//        MTLTextureSwizzleChannels swizzle = toMetalSwizzleChannels(desc.swizzle);
//        view_texture_ = [ref_tex newTextureViewWithPixelFormat:pixel_format
//                                                   textureType:texture_type
//                                                        levels:NSMakeRange(0, levels)
//                                                        slices:NSMakeRange(0, 1)
//                                                       swizzle:swizzle];
//    }
//}

} // namespace Pathfinder
