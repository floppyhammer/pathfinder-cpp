#include "command_encoder.h"

#include <cassert>
#include <cstring>

#include "buffer.h"
#include "compute_pipeline.h"
#include "debug_marker.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"
#include "sampler.h"

namespace Pathfinder {

CommandEncoderGl::~CommandEncoderGl() {
    invoke_callbacks();

    glDeleteVertexArrays(vao_.size(), vao_.data());
}

bool CommandEncoderGl::prepare() {
    if (commands_.empty()) {
        return false;
    }

    while (!commands_.empty()) {
        auto &cmd = commands_.front();

        switch (cmd.type) {
            case CommandType::BeginRenderPass: {
                assert(compute_pipeline_ == nullptr);

                auto &args = cmd.args.begin_render_pass;
                auto render_pass_gl = static_cast<RenderPassGl *>(args.render_pass);
                auto framebuffer_gl = static_cast<FramebufferGl *>(args.framebuffer);

                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_gl->get_gl_handle());

                if (render_pass_gl->get_attachment_load_op() == AttachmentLoadOp::Clear) {
                    glClearColor(args.clear_color.r_, args.clear_color.g_, args.clear_color.b_, args.clear_color.a_);
                    glClear(GL_COLOR_BUFFER_BIT);
                }

                gl_check_error("BeginRenderPass");
            } break;
            case CommandType::SetViewport: {
                auto &args = cmd.args.set_viewport;
                glViewport(args.viewport.min_x(), args.viewport.min_y(), args.viewport.width(), args.viewport.height());

                gl_check_error("SetViewport");
            } break;
            case CommandType::BindRenderPipeline: {
                auto &args = cmd.args.bind_render_pipeline;
                auto pipeline_gl = static_cast<RenderPipelineGl *>(args.pipeline);

                auto blend_state = pipeline_gl->get_blend_state();

                // Color blend.
                if (blend_state.enabled) {
                    glEnable(GL_BLEND);
                    glBlendFunc(to_gl_blend_factor(blend_state.color.src_factor),
                                to_gl_blend_factor(blend_state.color.dst_factor));
                } else {
                    glDisable(GL_BLEND);
                }

                pipeline_gl->get_program()->use();

                render_pipeline_ = args.pipeline;
                compute_pipeline_ = nullptr;

                gl_check_error("BindRenderPipeline");

                uint32_t vao;
                glGenVertexArrays(1, &vao);
                // We have to bind it here, in case we don't need any vertex buffer.
                glBindVertexArray(vao);
                vao_.push_back(vao);
                gl_check_error("GenVAO");
            } break;
            case CommandType::BindVertexBuffers: {
                assert(render_pipeline_ != nullptr);

                auto &args = cmd.args.bind_vertex_buffers;

                auto pipeline_gl = static_cast<RenderPipelineGl *>(render_pipeline_);

                auto buffer_count = args.buffer_count;
                auto vertex_buffers = args.buffers;
                auto vertex_offsets = args.offsets;

                assert(!vao_.empty() && "Must bind a render pipeline before binding vertex buffers!");
                glBindVertexArray(vao_.back());

                auto &attribute_descriptions = pipeline_gl->get_attribute_descriptions();

                uint32_t last_vbo = 0;

                for (int location = 0; location < attribute_descriptions.size(); location++) {
                    auto &attrib = attribute_descriptions[location];

                    if (attrib.binding >= buffer_count) {
                        assert("Vertex buffer binding exceeded buffer count!");
                        return false;
                    }

                    auto buffer = static_cast<BufferGl *>(vertex_buffers[attrib.binding]);
                    auto offset = vertex_offsets[attrib.binding];
                    auto vbo = buffer->get_handle();

                    if (location == 0) {
                        glBindBuffer(GL_ARRAY_BUFFER, vbo);
                    } else {
                        // If target VBO changed.
                        if (vbo != last_vbo) {
                            glBindBuffer(GL_ARRAY_BUFFER, vbo);
                        }
                    }

                    last_vbo = vbo;

                    switch (attrib.type) {
                        case DataType::i8:
                        case DataType::u8:
                        case DataType::i16:
                        case DataType::u16:
                        case DataType::i32:
                        case DataType::u32: {
                            glVertexAttribIPointer(location,
                                                   attrib.size,
                                                   to_gl_data_type(attrib.type),
                                                   (GLsizei)attrib.stride,
                                                   (void *)(offset + attrib.offset));
                        } break;
                        case DataType::f16:
                        case DataType::f32: {
                            glVertexAttribPointer(location,
                                                  attrib.size,
                                                  to_gl_data_type(attrib.type),
                                                  GL_FALSE,
                                                  (GLsizei)attrib.stride,
                                                  (void *)(offset + attrib.offset));
                        } break;
                    }

                    glEnableVertexAttribArray(location);

                    if (attrib.vertex_input_rate == VertexInputRate::Vertex) {
                        glVertexAttribDivisor(location, 0);
                    } else {
                        glVertexAttribDivisor(location, 1);
                    }
                }

                gl_check_error("BindVertexBuffers");
            } break;
            case CommandType::BindIndexBuffer: {
                const auto &args = cmd.args.bind_index_buffer;

                auto buffer_gl = (BufferGl *)args.buffer;
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_gl->get_handle());

                gl_check_error("BindIndexBuffer");
            } break;
            case CommandType::BindDescriptorSet: {
                auto &args = cmd.args.bind_descriptor_set;

                for (auto &pair : args.descriptor_set->get_descriptors()) {
                    auto &descriptor = pair.second;
                    auto binding_point = descriptor.binding;

                    switch (descriptor.type) {
                        case DescriptorType::UniformBuffer: {
                            auto buffer_gl = static_cast<BufferGl *>(descriptor.buffer.get());

                            glBindBufferRange(GL_UNIFORM_BUFFER,
                                              binding_point,
                                              buffer_gl->get_handle(),
                                              descriptor.buffer_offset,
                                              descriptor.buffer_range);

                            gl_check_error("bind uniform buffer");
                        } break;
                        case DescriptorType::Sampler: {
                            auto texture_gl = static_cast<TextureGl *>(descriptor.texture.get());

                            glActiveTexture(GL_TEXTURE0 + binding_point);
                            glBindTexture(GL_TEXTURE_2D, texture_gl->get_texture_id());

                            // Set sampler.
                            auto sampler_gl = static_cast<SamplerGl *>(descriptor.sampler.get());
                            glBindSampler(binding_point, sampler_gl->get_handle());

                            gl_check_error("bind texture");
                        } break;
#ifdef PATHFINDER_ENABLE_COMPUTE
                        case DescriptorType::StorageBuffer: {
                            auto buffer_gl = static_cast<BufferGl *>(descriptor.buffer.get());

                            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, buffer_gl->get_handle());
                        } break;
                        case DescriptorType::Image: {
                            auto texture_gl = static_cast<TextureGl *>(descriptor.texture.get());

                            glBindImageTexture(binding_point,
                                               texture_gl->get_texture_id(),
                                               0,
                                               GL_FALSE,
                                               0,
                                               GL_READ_WRITE,
                                               GL_RGBA8);
                        } break;
#endif
                        default:
                            break;
                    }
                }

                gl_check_error("BindDescriptorSet");
            } break;
            case CommandType::Draw: {
                auto &args = cmd.args.draw;

                glDrawArrays(GL_TRIANGLES, (GLsizei)args.first_vertex, (GLsizei)args.vertex_count);

                gl_check_error("Draw");
            } break;
            case CommandType::DrawIndexed: {
                auto &args = cmd.args.draw_indexed;

                glDrawElementsInstanced(GL_TRIANGLES,
                                        GLint(args.index_count),
                                        GL_UNSIGNED_INT,
                                        reinterpret_cast<void *>(args.first_index),
                                        args.instance_count);

                gl_check_error("DrawIndexed");
            } break;
            case CommandType::DrawInstanced: {
                auto &args = cmd.args.draw_instanced;

                glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)args.vertex_count, (GLsizei)args.instance_count);

                gl_check_error("DrawInstanced");
            } break;
#ifdef PATHFINDER_ENABLE_COMPUTE
            case CommandType::DrawIndirect: {
                auto &args = cmd.args.indirect;
                auto buffer_gl = (BufferGl *)args.buffer;

                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffer_gl->get_handle());
                glDrawArraysIndirect(GL_TRIANGLES, reinterpret_cast<void *>(args.offset));

                gl_check_error("DrawIndirect");
            } break;
            case CommandType::DispatchIndirect: {
                auto &args = cmd.args.indirect;
                auto buffer_gl = (BufferGl *)args.buffer;

                glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buffer_gl->get_handle());
                glDispatchComputeIndirect(static_cast<GLintptr>(args.offset));
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

                gl_check_error("DispatchIndirect");
            } break;
#endif
            case CommandType::EndRenderPass: {
                render_pipeline_ = nullptr;
            } break;
            case CommandType::BeginComputePass: {
                assert(render_pipeline_ == nullptr);
            } break;
            case CommandType::BindComputePipeline: {
                auto &args = cmd.args.bind_compute_pipeline;

                auto pipeline_gl = static_cast<ComputePipelineGl *>(args.pipeline);

                pipeline_gl->get_program()->use();

                compute_pipeline_ = args.pipeline;
            } break;
            case CommandType::Dispatch: {
#ifdef PATHFINDER_ENABLE_COMPUTE
                auto &args = cmd.args.dispatch;

                // Max global (total) work group counts x:2147483647 y:65535 z:65535.
                // Max local (in one shader) work group sizes x:1536 y:1024 z:64.
                glDispatchCompute(args.group_size_x, args.group_size_y, args.group_size_z);

    // In order to use timestamps more precisely.
    #ifndef NDEBUG
                glFinish();
    #endif

                gl_check_error("Dispatch");
#endif
            } break;
            case CommandType::EndComputePass: {
                compute_pipeline_ = nullptr;
            } break;
            case CommandType::WriteBuffer: {
                auto &args = cmd.args.write_buffer;

                args.buffer->upload_via_mapping(args.data_size, args.offset, args.data);
            } break;
            case CommandType::ReadBuffer: {
                auto &args = cmd.args.read_buffer;

                args.buffer->download_via_mapping(args.data_size, args.offset, args.data);
            } break;
            case CommandType::WriteTexture: {
                const auto &args = cmd.args.write_texture;

                auto texture_gl = static_cast<TextureGl *>(args.texture);
                texture_gl->prepare_pbo();

                const GLsizeiptr region_data_size = (args.width - args.offset_x) * (args.height - args.offset_y) *
                                                    get_pixel_size(texture_gl->get_format());

                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, texture_gl->get_pbo_id());

                void *ptr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER,
                                             0,
                                             region_data_size,
                                             GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                if (ptr) {
                    memcpy(ptr, args.data, region_data_size);
                    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                }

                glBindTexture(GL_TEXTURE_2D, texture_gl->get_texture_id());
                glTexSubImage2D(GL_TEXTURE_2D,
                                0,
                                (GLint)args.offset_x,
                                (GLint)args.offset_y,
                                (GLint)args.width,
                                (GLint)args.height,
                                to_gl_pixel_data_format(args.texture->get_format()),
                                to_gl_data_type(texture_format_to_data_type(args.texture->get_format())),
                                0);

                // Unbind.
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

                gl_check_error("WriteTexture");
            } break;
            case CommandType::ReadTexture: {
                const auto &args = cmd.args.read_texture;

                auto texture_gl = static_cast<TextureGl *>(args.texture);
                texture_gl->prepare_pbo();

                const GLsizeiptr region_data_size = (args.width - args.offset_x) * (args.height - args.offset_y) *
                                                    get_pixel_size(texture_gl->get_format());

                glBindBuffer(GL_PIXEL_PACK_BUFFER, texture_gl->get_pbo_id());

                glReadPixels(args.offset_x,
                             args.offset_y,
                             args.width,
                             args.height,
                             to_gl_pixel_data_format(args.texture->get_format()),
                             to_gl_data_type(texture_format_to_data_type(args.texture->get_format())),
                             0);

                void *ptr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, region_data_size, GL_MAP_READ_BIT);
                if (ptr) {
                    memcpy(args.data, ptr, region_data_size);
                    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                }

                // Unbind.
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

                gl_check_error("ReadTexture");
            } break;
            case CommandType::Max:
                break;
        }

        commands_.pop_front();
    }

    return true;
}

} // namespace Pathfinder
