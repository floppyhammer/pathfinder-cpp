#pragma once

#include <MetalKit/MetalKit.h>

#include "../command_encoder.h"

namespace Pathfinder {

class MetalSwapChain;

class RenderPipelineMtl;

class ComputePipelineMtl;

class TextureMtl;

class BufferMtl;

class MTKViewSwapChain;

class CommandEncoderMtl final : public CommandEncoder {
    friend class MTKViewSwapChain;
    friend class QueueMtl;
    friend class DeviceMtl;
    friend class BufferMtl;
    friend class TextureMtl;

public:
    id<MTLCommandBuffer> get_handle() noexcept {
        return mtl_cmd_buffer_;
    }

private:
    CommandEncoderMtl(id<MTLDevice> mtl_device, id<MTLCommandQueue> mtl_cmd_queue)
        : mtl_device_(mtl_device), mtl_cmd_queue_(mtl_cmd_queue), mtl_cmd_buffer_(nil) {}

    bool finish() override;

    std::vector<std::function<void()>> callbacks;

    void add_callback(const std::function<void()> &callback) {
        callbacks.push_back(callback);
    }

    void clear_pending_ops() {
        for (auto &callback : callbacks) {
            callback();
        }
        callbacks.clear();
    }

    id<MTLDevice> mtl_device_ = nil;
    id<MTLCommandQueue> mtl_cmd_queue_ = nil;
    id<MTLCommandBuffer> mtl_cmd_buffer_ = nil;
};

} // namespace Pathfinder
