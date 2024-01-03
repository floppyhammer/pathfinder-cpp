#include "command_encoder.h"

#include <cassert>

#include "buffer.h"
#include "compute_pipeline.h"
#include "debug_marker.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"

namespace Pathfinder {

CommandEncoderGl::~CommandEncoderGl() {
    invoke_callbacks();
}

bool CommandEncoderGl::finish() {
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

                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_gl->get_gl_framebuffer());

                if (render_pass_gl->get_attachment_load_op() == AttachmentLoadOp::Clear) {
                    glClearColor(args.clear_color.r_, args.clear_color.g_, args.clear_color.b_, args.clear_color.a_);
                    glClear(GL_COLOR_BUFFER_BIT);
                }

                glViewport(args.viewport.min_x(), args.viewport.min_y(), args.viewport.max_x(), args.viewport.max_y());

                gl_check_error("BeginRenderPass");
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
            } break;
            case CommandType::BindVertexBuffers: {
                assert(render_pipeline_ != nullptr);

                auto &args = cmd.args.bind_vertex_buffers;

                auto pipeline_gl = static_cast<RenderPipelineGl *>(render_pipeline_);

                auto buffer_count = args.buffer_count;
                auto vertex_buffers = args.buffers;

                glBindVertexArray(pipeline_gl->get_vao());

                auto &attribute_descriptions = pipeline_gl->get_attribute_descriptions();

                uint32_t last_vbo = 0;

                for (int location = 0; location < attribute_descriptions.size(); location++) {
                    auto &attrib = attribute_descriptions[location];

                    if (attrib.binding >= buffer_count) {
                        assert("Vertex buffer binding exceeded buffer count!");
                        return false;
                    }

                    auto buffer = static_cast<BufferGl *>(vertex_buffers[attrib.binding]);
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
                                                   (void *)attrib.offset);
                        } break;
                        case DataType::f16:
                        case DataType::f32: {
                            glVertexAttribPointer(location,
                                                  attrib.size,
                                                  to_gl_data_type(attrib.type),
                                                  GL_FALSE,
                                                  (GLsizei)attrib.stride,
                                                  (void *)attrib.offset);
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
            case CommandType::BindDescriptorSet: {
                auto &args = cmd.args.bind_descriptor_set;

                uint32_t program_id;
                if (render_pipeline_ != nullptr) {
                    auto pipeline_gl = static_cast<RenderPipelineGl *>(render_pipeline_);
                    program_id = pipeline_gl->get_program()->get_handle();
                } else {
                    auto pipeline_gl = static_cast<ComputePipelineGl *>(compute_pipeline_);
                    program_id = pipeline_gl->get_program()->get_handle();
                }

                for (auto &pair : args.descriptor_set->get_descriptors()) {
                    auto &descriptor = pair.second;

                    // Note that pair.first is not the binding point.
                    auto binding_point = descriptor.binding;
                    auto binding_name = descriptor.binding_name;

                    switch (descriptor.type) {
                        case DescriptorType::UniformBuffer: {
                            auto buffer_gl = static_cast<BufferGl *>(descriptor.buffer.get());

                            unsigned int ubo_index = glGetUniformBlockIndex(program_id, binding_name.c_str());
                            glUniformBlockBinding(program_id, ubo_index, binding_point);
                            glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, buffer_gl->get_handle());

                            gl_check_error("Mismatched uniform binding name!");
                        } break;
                        case DescriptorType::Sampler: {
                            auto texture_gl = static_cast<TextureGl *>(descriptor.texture.get());

                            if (!binding_name.empty()) {
                                glUniform1i(glGetUniformLocation(program_id, binding_name.c_str()),
                                            (GLint)binding_point);

                                gl_check_error("Mismatched texture binding name!");
                            }
                            glActiveTexture(GL_TEXTURE0 + binding_point);

                            auto sampler_descriptor = descriptor.sampler->get_descriptor();

                            glBindTexture(GL_TEXTURE_2D, texture_gl->get_texture_id());

                            // Set texture sampler.
                            // --------------------------------------------------------------
                            // Set wrapping parameters.
                            glTexParameteri(GL_TEXTURE_2D,
                                            GL_TEXTURE_WRAP_S,
                                            to_gl_sampler_address_mode(sampler_descriptor.address_mode_u));
                            glTexParameteri(GL_TEXTURE_2D,
                                            GL_TEXTURE_WRAP_T,
                                            to_gl_sampler_address_mode(sampler_descriptor.address_mode_v));

                            // Set filtering parameters.
                            glTexParameteri(GL_TEXTURE_2D,
                                            GL_TEXTURE_MIN_FILTER,
                                            to_gl_sampler_filter(sampler_descriptor.min_filter));
                            glTexParameteri(GL_TEXTURE_2D,
                                            GL_TEXTURE_MAG_FILTER,
                                            to_gl_sampler_filter(sampler_descriptor.mag_filter));
                            // --------------------------------------------------------------
                        } break;
#ifdef PATHFINDER_ENABLE_D3D11
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
            case CommandType::DrawInstanced: {
                auto &args = cmd.args.draw_instanced;

                glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)args.vertex_count, (GLsizei)args.instance_count);

                gl_check_error("DrawInstanced");
            } break;
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
#ifdef PATHFINDER_ENABLE_D3D11
                auto &args = cmd.args.dispatch;

                // Max global (total) work group counts x:2147483647 y:65535 z:65535.
                // Max local (in one shader) work group sizes x:1536 y:1024 z:64.
                glDispatchCompute(args.group_size_x, args.group_size_y, args.group_size_z);

                // In order to use timestamps more precisely.
    #ifdef PATHFINDER_DEBUG
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
                auto &args = cmd.args.write_texture;

                auto texture_gl = static_cast<TextureGl *>(args.texture);

                glBindTexture(GL_TEXTURE_2D, texture_gl->get_texture_id());
                glTexSubImage2D(GL_TEXTURE_2D,
                                0,
                                (GLint)args.offset_x,
                                (GLint)args.offset_y,
                                (GLint)args.width,
                                (GLint)args.height,
                                GL_RGBA,
                                to_gl_data_type(texture_format_to_data_type(args.texture->get_format())),
                                args.data);
                glBindTexture(GL_TEXTURE_2D, 0); // Unbind.

                gl_check_error("WriteTexture");
            } break;
            case CommandType::ReadTexture: {
                auto &args = cmd.args.read_texture;

                auto texture_gl = static_cast<TextureGl *>(args.texture);

                GLenum temp_fbo;
                glGenFramebuffers(1, &temp_fbo);
                glBindFramebuffer(GL_FRAMEBUFFER, temp_fbo);

                glFramebufferTexture2D(GL_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_2D,
                                       texture_gl->get_texture_id(),
                                       0);

                glReadPixels(args.offset_x,
                             args.offset_y,
                             args.width,
                             args.height,
                             GL_RGBA,
                             to_gl_data_type(texture_format_to_data_type(args.texture->get_format())),
                             args.data);

                glBindTexture(GL_FRAMEBUFFER, 0); // Unbind.
                glDeleteFramebuffers(1, &temp_fbo);

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
