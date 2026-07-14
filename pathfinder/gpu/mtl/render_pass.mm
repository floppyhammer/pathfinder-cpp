#include "render_pass.h"

namespace Pathfinder {

RenderPassMtl::~RenderPassMtl() {}

RenderPassMtl::RenderPassMtl(TextureFormat texture_format,
                             AttachmentLoadOp load_op,
                             bool is_swap_chain_pass,
                             const std::string& label) {
    load_op_ = load_op;
    label_ = label;
    mtl_render_pass_desc_ = [MTLRenderPassDescriptor new];
}

} // namespace Pathfinder
