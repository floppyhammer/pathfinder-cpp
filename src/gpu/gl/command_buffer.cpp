#include "command_buffer.h"

#include <cassert>

#include "buffer.h"
#include "compute_pipeline.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"
#include "validation.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

void CommandBufferGl::upload_to_buffer(const std::shared_ptr<Buffer> &buffer,
                                       uint32_t offset,
                                       uint32_t data_size,
                                       void *data) {
    if (data_size == 0 || data == nullptr) {
        Logger::error("Tried to upload invalid data to buffer!");
    }

    Command cmd;
    cmd.type = CommandType::UploadToBuffer;

    auto &args = cmd.args.upload_to_buffer;
    args.buffer = buffer.get();
    args.offset = offset;
    args.data_size = data_size;
    args.data = data;

    commands.push(cmd);

    // Moved from submit() to here in order to handle host-visible/coherent buffers properly.
    // -------------------------------------------------------
    auto buffer_gl = static_cast<BufferGl *>(args.buffer);

    int gl_buffer_type;

    switch (args.buffer->type) {
        case BufferType::Vertex: {
            gl_buffer_type = GL_ARRAY_BUFFER;
        } break;
        case BufferType::Uniform: {
            gl_buffer_type = GL_UNIFORM_BUFFER;
        } break;
    #ifdef PATHFINDER_USE_D3D11
        case BufferType::Storage: {
            gl_buffer_type = GL_SHADER_STORAGE_BUFFER;
        } break;
    #endif
    }

    glBindBuffer(gl_buffer_type, buffer_gl->id);
    glBufferSubData(gl_buffer_type, args.offset, args.data_size, args.data);
    glBindBuffer(gl_buffer_type, 0); // Unbind.

    check_error("UploadToBuffer");
    // -------------------------------------------------------
}

void CommandBufferGl::submit(const std::shared_ptr<Driver> &_driver) {
    while (!commands.empty()) {
        auto &cmd = commands.front();

        switch (cmd.type) {
            case CommandType::BeginRenderPass: {
                auto &args = cmd.args.begin_render_pass;
                auto render_pass_gl = static_cast<RenderPassGl *>(args.render_pass);
                auto framebuffer_gl = static_cast<FramebufferGl *>(args.framebuffer);

                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_gl->get_gl_framebuffer());

                if (render_pass_gl->get_attachment_load_op() == AttachmentLoadOp::Clear) {
                    glClearColor(args.clear_color.r, args.clear_color.g, args.clear_color.b, args.clear_color.a);
                    glClear(GL_COLOR_BUFFER_BIT);
                }

                glViewport(0, 0, args.extent.x, args.extent.y);

                check_error("BeginRenderPass");
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

                render_pipeline = args.pipeline;
                compute_pipeline = nullptr;

                check_error("BindRenderPipeline");
            } break;
            case CommandType::BindVertexBuffers: {
                auto pipeline_gl = static_cast<RenderPipelineGl *>(render_pipeline);

                auto &args = cmd.args.bind_vertex_buffers;

                auto buffer_count = args.buffer_count;
                auto vertex_buffers = args.buffers;

                glBindVertexArray(pipeline_gl->get_vao());

                auto &attribute_descriptions = pipeline_gl->get_attribute_descriptions();

                auto last_vbo = 0;
                for (int location = 0; location < attribute_descriptions.size(); location++) {
                    auto &attrib = attribute_descriptions[location];

                    if (attrib.binding >= buffer_count) {
                        assert("Vertex buffer binding exceeded buffer count!");
                        return;
                    }

                    auto buffer = static_cast<BufferGl *>(vertex_buffers[attrib.binding]);
                    auto vbo = buffer->id;

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
                                                   attrib.stride,
                                                   (void *)attrib.offset);
                        } break;
                        case DataType::f16:
                        case DataType::f32: {
                            glVertexAttribPointer(location,
                                                  attrib.size,
                                                  to_gl_data_type(attrib.type),
                                                  GL_FALSE,
                                                  attrib.stride,
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

                check_error("BindVertexBuffers");
            } break;
            case CommandType::BindDescriptorSet: {
                auto &args = cmd.args.bind_descriptor_set;

                uint32_t program_id;
                if (render_pipeline != nullptr) {
                    auto pipeline_gl = static_cast<RenderPipelineGl *>(render_pipeline);
                    program_id = pipeline_gl->get_program()->get_id();
                } else {
                    auto pipeline_gl = static_cast<ComputePipelineGl *>(compute_pipeline);
                    program_id = pipeline_gl->get_program()->get_id();
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
                            glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, buffer_gl->id);
                        } break;
                        case DescriptorType::Sampler: {
                            auto texture_gl = static_cast<TextureGl *>(descriptor.texture.get());

                            if (!binding_name.empty()) {
                                glUniform1i(glGetUniformLocation(program_id, binding_name.c_str()), binding_point);
                            }
                            glActiveTexture(GL_TEXTURE0 + binding_point);
                            glBindTexture(GL_TEXTURE_2D, texture_gl->get_texture_id());
                        } break;
    #ifdef PATHFINDER_USE_D3D11
                        case DescriptorType::StorageBuffer: {
                            auto buffer_gl = static_cast<BufferGl *>(descriptor.buffer.get());

                            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, buffer_gl->id);
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

                check_error("BindDescriptorSet");
            } break;
            case CommandType::Draw: {
                auto &args = cmd.args.draw;

                glDrawArrays(GL_TRIANGLES, (GLsizei)args.first_vertex, (GLsizei)args.vertex_count);

                check_error("Draw");
            } break;
            case CommandType::DrawInstanced: {
                auto &args = cmd.args.draw_instanced;

                glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)args.vertex_count, (GLsizei)args.instance_count);

                check_error("DrawInstanced");
            } break;
            case CommandType::EndRenderPass:
            case CommandType::BeginComputePass:
                break;
            case CommandType::BindComputePipeline: {
                auto &args = cmd.args.bind_compute_pipeline;

                auto pipeline_gl = static_cast<ComputePipelineGl *>(args.pipeline);

                pipeline_gl->get_program()->use();

                render_pipeline = nullptr;
                compute_pipeline = args.pipeline;
            } break;
            case CommandType::Dispatch: {
                auto &args = cmd.args.dispatch;

    #ifdef PATHFINDER_USE_D3D11
                // Max global (total) work group counts x:2147483647 y:65535 z:65535.
                // Max local (in one shader) work group sizes x:1536 y:1024 z:64.
                glDispatchCompute(args.group_size_x, args.group_size_y, args.group_size_z);

                    // In order to use timestamps more precisely.
        #ifdef PATHFINDER_DEBUG
                glFinish();
        #endif
    #endif

                check_error("Dispatch");
            } break;
            case CommandType::EndComputePass: {
            } break;
            case CommandType::UploadToBuffer: {
            } break;
            case CommandType::ReadBuffer: {
                auto &args = cmd.args.read_buffer;

                // We can only read from general buffers.
                if (args.buffer->type != BufferType::Storage) {
                    Logger::error("Tried to read from a non-general buffer!", "Command Buffer");
                    break;
                }

    #ifdef PATHFINDER_USE_D3D11
                auto buffer_gl = static_cast<BufferGl *>(args.buffer);

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_gl->id);

        #ifdef __ANDROID__
                void *ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, args.offset, args.data_size, GL_MAP_READ_BIT);
                if (ptr) memcpy(args.data, ptr, args.data_size);
                glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        #else
                glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, args.offset, args.data_size, args.data);
        #endif

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind.
    #endif

                check_error("ReadBuffer");
            } break;
            case CommandType::UploadToTexture: {
                auto &args = cmd.args.upload_to_texture;

                auto texture_gl = static_cast<TextureGl *>(args.texture);

                glBindTexture(GL_TEXTURE_2D, texture_gl->get_texture_id());
                glTexSubImage2D(GL_TEXTURE_2D,
                                0,
                                args.offset_x,
                                args.offset_y,
                                args.width,
                                args.height,
                                GL_RGBA,
                                to_gl_data_type(texture_format_to_data_type(args.texture->get_format())),
                                args.data);
                glBindTexture(GL_TEXTURE_2D, 0); // Unbind.

                check_error("UploadToTexture");
            } break;
            case CommandType::Max:
                break;
        }

        commands.pop();
    }

    // Release allocated memory if there's any.
    for (auto &callback : callbacks) {
        callback();
    }

    callbacks.clear();
}

} // namespace Pathfinder

#endif
