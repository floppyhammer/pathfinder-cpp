#pragma once

#include "../render_pass.h"
#include "base.h"

namespace Pathfinder {

class RenderPassMtl final : public RenderPass {
    friend class DeviceMtl;

public:
    ~RenderPassMtl() override;

    MTLRenderPassDescriptor *get_mtl_render_pass_desc() {
        return mtl_render_pass_desc_;
    }

private:
    RenderPassMtl(TextureFormat texture_format,
                  AttachmentLoadOp load_op,
                  bool is_swap_chain_pass,
                  const std::string &label);

    MTLRenderPassDescriptor *mtl_render_pass_desc_ = nullptr;
};

} // namespace Pathfinder
