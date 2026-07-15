#include "command_encoder.h"

#include <cassert>

#include "../base.h"
#include "base.h"
#include "buffer.h"
#include "compute_pipeline.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"
#include "sampler.h"
#include "texture.h"

namespace Pathfinder {

bool CommandEncoderMtl::prepare() {
    if (commands_.empty()) {
        return false;
    }

    assert(mtl_cmd_buffer_ == nil);
    mtl_cmd_buffer_ = [mtl_cmd_queue_ commandBuffer];

    id<MTLRenderCommandEncoder> current_render_cmd_encoder_ = nil;
    id<MTLComputeCommandEncoder> current_compute_cmd_encoder_ = nil;

    // Temporary states.
    BufferMtl *current_index_buffer{};
    MTLIndexType current_index_type = MTLIndexTypeUInt32;
    MTLPrimitiveType current_primitive_type = MTLPrimitiveTypeTriangle;

    for (const auto &cmd : commands_) {
        switch (cmd.type) {
            case CommandType::BeginRenderPass: {
                auto &args = cmd.args.begin_render_pass;

                auto framebuffer_mtl = (FramebufferMtl *)args.framebuffer;
                auto render_pass_mtl = (RenderPassMtl *)args.render_pass;
                auto render_pass_desc = render_pass_mtl->get_mtl_render_pass_desc();

                if (render_pass_mtl->get_attachment_load_op() == AttachmentLoadOp::Clear) {
                    render_pass_desc.colorAttachments[0].loadAction = MTLLoadActionClear;
                    render_pass_desc.colorAttachments[0].clearColor = MTLClearColorMake(args.clear_color.r_,
                                                                                        args.clear_color.g_,
                                                                                        args.clear_color.b_,
                                                                                        args.clear_color.a_);
                } else {
                    render_pass_desc.colorAttachments[0].loadAction = MTLLoadActionLoad;
                }

                render_pass_desc.colorAttachments[0].storeAction = MTLStoreActionStore;

                auto texture_mtl = (TextureMtl *)framebuffer_mtl->get_texture().get();
                render_pass_desc.colorAttachments[0].texture = texture_mtl->get_handle();

                // If the texture has a depth attachment or needs one, it should be configured here.
                // For now, we assume color-only as per the existing code.

                assert(current_render_cmd_encoder_ == nil);
                current_render_cmd_encoder_ = [mtl_cmd_buffer_ renderCommandEncoderWithDescriptor:render_pass_desc];
            } break;
            case CommandType::SetViewport: {
                const auto &viewport = cmd.args.set_viewport.viewport;

                auto mtl_viewport = MTLViewport{static_cast<double>(viewport.left),
                                                static_cast<double>(viewport.top),
                                                static_cast<double>(viewport.width()),
                                                static_cast<double>(viewport.height()),
                                                0.0,
                                                1.0};

                [current_render_cmd_encoder_ setViewport:mtl_viewport];
            } break;
            case CommandType::BindVertexBuffers: {
                const auto &args = cmd.args.bind_vertex_buffers;

                for (uint32_t i = 0; i < args.buffer_count; i++) {
                    auto buffer_mtl = static_cast<BufferMtl *>(args.buffers[i]);

                    [current_render_cmd_encoder_ setVertexBuffer:buffer_mtl->get_handle()
                                                          offset:args.offsets[i]
                                                         atIndex:i];
                }
            } break;
            case CommandType::BindIndexBuffer: {
                const auto &args = cmd.args.bind_index_buffer;
                current_index_buffer = (BufferMtl *)args.buffer;
                current_index_type = (args.data_type == DataType::u16) ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
            } break;
            case CommandType::BindRenderPipeline: {
                const auto &args = cmd.args.bind_render_pipeline;
                auto pipeline_mtl = (RenderPipelineMtl *)args.pipeline;

                [current_render_cmd_encoder_ setRenderPipelineState:pipeline_mtl->get_handle()];
                if (pipeline_mtl->get_depth_state() != nil) {
                    [current_render_cmd_encoder_ setDepthStencilState:pipeline_mtl->get_depth_state()];
                }

                [current_render_cmd_encoder_ setCullMode:MTLCullModeNone];
            } break;
            case CommandType::Draw: {
                [current_render_cmd_encoder_ drawPrimitives:current_primitive_type
                                                vertexStart:cmd.args.draw.first_vertex
                                                vertexCount:cmd.args.draw.vertex_count];
            } break;
            case CommandType::DrawInstanced: {
                [current_render_cmd_encoder_ drawPrimitives:current_primitive_type
                                                vertexStart:0
                                                vertexCount:cmd.args.draw_instanced.vertex_count
                                              instanceCount:cmd.args.draw_instanced.instance_count
                                               baseInstance:0];
            } break;
            case CommandType::DrawIndexed: {
                const auto &args = cmd.args.draw_indexed;
                if (!current_index_buffer) break;

                const size_t index_size = (current_index_type == MTLIndexTypeUInt16 ? 2 : 4);
                const size_t index_offset = args.first_index * index_size;

                [current_render_cmd_encoder_ drawIndexedPrimitives:current_primitive_type
                                                        indexCount:args.index_count
                                                         indexType:current_index_type
                                                       indexBuffer:current_index_buffer->get_handle()
                                                 indexBufferOffset:index_offset
                                                     instanceCount:args.instance_count
                                                        baseVertex:0
                                                      baseInstance:args.first_instance];
            } break;
            case CommandType::DrawIndirect: {
                const auto &args = cmd.args.indirect;
                auto buffer_mtl = (BufferMtl *)args.buffer;

                [current_render_cmd_encoder_ drawPrimitives:current_primitive_type
                                             indirectBuffer:buffer_mtl->get_handle()
                                       indirectBufferOffset:args.offset];
            } break;
            case CommandType::EndRenderPass: {
                [current_render_cmd_encoder_ endEncoding];
                current_render_cmd_encoder_ = nil;
            } break;
            case CommandType::BeginComputePass: {
                assert(current_compute_cmd_encoder_ == nil);
                current_compute_cmd_encoder_ = [mtl_cmd_buffer_ computeCommandEncoder];
            } break;
            case CommandType::BindComputePipeline: {
                auto pipeline_mtl = (ComputePipelineMtl *)cmd.args.bind_compute_pipeline.pipeline;
                [current_compute_cmd_encoder_ setComputePipelineState:pipeline_mtl->get_handle()];
            } break;
            case CommandType::Dispatch: {
                const auto &args = cmd.args.dispatch;

                auto group_size = MTLSizeMake(args.group_size_x, args.group_size_y, args.group_size_z);
                // FIXME: threadsPerThreadgroup should be dynamically retrieved from shdbin.
                auto subgroup_size = MTLSizeMake(64, 1, 1);

                [current_compute_cmd_encoder_ dispatchThreadgroups:group_size threadsPerThreadgroup:subgroup_size];
            } break;
            case CommandType::DispatchIndirect: {
            } break;
            case CommandType::WriteTexture: {
                id<MTLBlitCommandEncoder> blit_encoder = [mtl_cmd_buffer_ blitCommandEncoder];

                const auto &args = cmd.args.write_texture;
                auto texture_mtl = (TextureMtl *)args.texture;
                auto staging_buffer_mtl = (BufferMtl *)args.staging_buffer;

                auto pixel_size = get_pixel_size(texture_mtl->get_format());

                [blit_encoder copyFromBuffer:staging_buffer_mtl->get_handle()
                                sourceOffset:args.staging_offset
                           sourceBytesPerRow:args.width * pixel_size
                         sourceBytesPerImage:0
                                  sourceSize:MTLSizeMake(args.width, args.height, 1)
                                   toTexture:texture_mtl->get_handle()
                            destinationSlice:0
                            destinationLevel:0
                           destinationOrigin:MTLOriginMake(args.offset_x, args.offset_y, 0)
                                     options:MTLBlitOptionNone];

                [blit_encoder endEncoding];
                blit_encoder = nil;
            } break;
            case CommandType::WriteBuffer: {
                id<MTLBlitCommandEncoder> blit_encoder = [mtl_cmd_buffer_ blitCommandEncoder];

                const auto &args = cmd.args.write_buffer;
                auto buffer_mtl = (BufferMtl *)args.buffer;
                auto staging_buffer_mtl = (BufferMtl *)args.staging_buffer;

                [blit_encoder copyFromBuffer:staging_buffer_mtl->get_handle()
                                sourceOffset:args.staging_offset
                                    toBuffer:buffer_mtl->get_handle()
                           destinationOffset:args.offset
                                        size:args.data_size];

                [blit_encoder endEncoding];
                blit_encoder = nil;
            } break;
            case CommandType::ReadTexture: {
                id<MTLBlitCommandEncoder> blit_encoder = [mtl_cmd_buffer_ blitCommandEncoder];

                const auto &args = cmd.args.read_texture;
                auto texture_mtl = (TextureMtl *)args.texture;
                auto staging_buffer_mtl = (BufferMtl *)args.staging_buffer;

                auto pixel_size = get_pixel_size(texture_mtl->get_format());
                uint32_t bytes_per_row = args.width * pixel_size;
                uint32_t data_size = bytes_per_row * args.height;

                [blit_encoder copyFromTexture:texture_mtl->get_handle()
                                  sourceSlice:0
                                  sourceLevel:0
                                 sourceOrigin:MTLOriginMake(args.offset_x, args.offset_y, 0)
                                   sourceSize:MTLSizeMake(args.width, args.height, 1)
                                     toBuffer:staging_buffer_mtl->get_handle()
                            destinationOffset:args.staging_offset
                       destinationBytesPerRow:bytes_per_row
                     destinationBytesPerImage:data_size];

                [blit_encoder endEncoding];
                blit_encoder = nil;
            } break;
            case CommandType::ReadBuffer: {
                const auto &args = cmd.args.read_buffer;
                auto buffer_mtl = (BufferMtl *)args.buffer;
                auto staging_buffer_mtl = (BufferMtl *)args.staging_buffer;

                id<MTLBlitCommandEncoder> blit_encoder = [mtl_cmd_buffer_ blitCommandEncoder];

                [blit_encoder copyFromBuffer:buffer_mtl->get_handle()
                                sourceOffset:args.offset
                                    toBuffer:staging_buffer_mtl->get_handle()
                           destinationOffset:args.staging_offset
                                        size:args.data_size];

                [blit_encoder endEncoding];
                blit_encoder = nil;
            } break;
            case CommandType::BindDescriptorSet: {
                const auto &args = cmd.args.bind_descriptor_set;
                const auto set_layout = args.descriptor_set->get_layout();

                // Bind resources (uniform buffers, texture samplers, images, etc.).
                for (auto item : args.descriptor_set->get_descriptors()) {
                    uint32_t binding_point = item.first;
                    const auto &d = item.second;

                    auto descriptor_layout = set_layout->get_descriptor_layout(binding_point);
                    auto stage = descriptor_layout.stage;

                    // Uniform.
                    if (d.type == DescriptorType::UniformBuffer) {
                        auto buffer_mtl = static_cast<BufferMtl *>(d.buffer.get());
                        auto mtl_buffer = buffer_mtl->get_handle();

                        // Avoid the first 8 vertex buffer locations.
                        if (stage == ShaderStage::Vertex || stage == ShaderStage::VertexAndFragment) {
                            [current_render_cmd_encoder_ setVertexBuffer:mtl_buffer
                                                                  offset:d.buffer_offset
                                                                 atIndex:binding_point + 8];
                        }
                        if (stage == ShaderStage::Fragment || stage == ShaderStage::VertexAndFragment) {
                            [current_render_cmd_encoder_ setFragmentBuffer:mtl_buffer
                                                                    offset:d.buffer_offset
                                                                   atIndex:binding_point + 8];
                        }
                        if (stage == ShaderStage::Compute) {
                            [current_compute_cmd_encoder_ setBuffer:mtl_buffer
                                                             offset:d.buffer_offset
                                                            atIndex:binding_point];
                        }
                    } else if (d.type == DescriptorType::Sampler) {
                        auto texture_mtl = static_cast<TextureMtl *>(d.texture.get());
                        auto sampler_mtl = static_cast<SamplerMtl *>(d.sampler.get());

                        if (stage == ShaderStage::Vertex || stage == ShaderStage::VertexAndFragment) {
                            [current_render_cmd_encoder_ setVertexTexture:texture_mtl->get_handle()
                                                                  atIndex:binding_point];
                            [current_render_cmd_encoder_ setVertexSamplerState:sampler_mtl->get_handle()
                                                                       atIndex:binding_point];
                        }
                        if (stage == ShaderStage::Fragment || stage == ShaderStage::VertexAndFragment) {
                            [current_render_cmd_encoder_ setFragmentTexture:texture_mtl->get_handle()
                                                                    atIndex:binding_point];
                            [current_render_cmd_encoder_ setFragmentSamplerState:sampler_mtl->get_handle()
                                                                         atIndex:binding_point];
                        }
                        if (stage == ShaderStage::Compute) {
                            [current_compute_cmd_encoder_ setTexture:texture_mtl->get_handle() atIndex:binding_point];
                            [current_compute_cmd_encoder_ setSamplerState:sampler_mtl->get_handle()
                                                                  atIndex:binding_point];
                        }
                    } else if (d.type == DescriptorType::Image) {
                        auto texture_mtl = static_cast<TextureMtl *>(d.texture.get());
                        [current_compute_cmd_encoder_ setTexture:texture_mtl->get_handle() atIndex:binding_point];
                    } else if (d.type == DescriptorType::StorageBuffer) {
                        auto buffer_mtl = static_cast<BufferMtl *>(d.buffer.get());
                        [current_compute_cmd_encoder_ setBuffer:buffer_mtl->get_handle()
                                                         offset:d.buffer_offset
                                                        atIndex:binding_point];
                    }
                }
            } break;
            case CommandType::EndComputePass: {
                [current_compute_cmd_encoder_ endEncoding];
                current_compute_cmd_encoder_ = nil;
            } break;
        }
    }

    return true;
}

} // namespace Pathfinder
