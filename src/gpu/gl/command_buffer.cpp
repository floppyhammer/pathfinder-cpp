#include "command_buffer.h"

#include "framebuffer.h"
#include "render_pipeline.h"
#include "compute_pipeline.h"
#include "buffer.h"
#include "validation.h"

#include <cassert>

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    void CommandBufferGl::begin_render_pass(const std::shared_ptr<RenderPass> &render_pass,
                                            const std::shared_ptr<Framebuffer> &framebuffer,
                                            bool clear,
                                            ColorF clear_color) {
        Command cmd;
        cmd.type = CommandType::BeginRenderPass;

        auto &args = cmd.args.begin_render_pass;
        args.render_pass = render_pass.get();
        args.framebuffer = framebuffer.get();
        args.extent = {framebuffer->get_width(), framebuffer->get_height()};
        args.clear = clear;
        args.clear_color = clear_color;

        commands.push(cmd);
    }

    void CommandBuffer::end_render_pass() {

    }

    void CommandBufferGl::bind_render_pipeline(const std::shared_ptr<RenderPipeline> &pipeline) {
        Command cmd;
        cmd.type = CommandType::BindRenderPipeline;
        auto &args = cmd.args.bind_render_pipeline;
        args.pipeline = pipeline.get();

        commands.push(cmd);
    }

    void CommandBufferGl::bind_compute_pipeline(const std::shared_ptr<ComputePipeline> &pipeline) {
        Command cmd;
        cmd.type = CommandType::BindComputePipeline;
        auto &args = cmd.args.bind_compute_pipeline;
        args.pipeline = pipeline.get();

        commands.push(cmd);
    }

    void CommandBufferGl::bind_vertex_buffers(std::vector<std::shared_ptr<Buffer>> vertex_buffers) {
        Command cmd;
        cmd.type = CommandType::BindVertexBuffers;
        auto &args = cmd.args.bind_vertex_buffers;
        args.buffer_count = vertex_buffers.size();

        assert(vertex_buffers.size() < 10 && "Maximum vertex buffer per pipeline is 10!");
        for (int buffer_index = 0; buffer_index < vertex_buffers.size(); buffer_index++) {
            args.buffers[buffer_index] = vertex_buffers[buffer_index].get();
        }

        commands.push(cmd);
    }

    void CommandBufferGl::bind_descriptor_set(const std::shared_ptr<DescriptorSet> &descriptor_set) {
        Command cmd;
        cmd.type = CommandType::BindDescriptorSet;

        auto &args = cmd.args.bind_descriptor_set;
        args.descriptor_set = descriptor_set.get();

        commands.push(cmd);
    }

    void CommandBufferGl::draw(uint32_t first_vertex, uint32_t vertex_count) {
        Command cmd;
        cmd.type = CommandType::Draw;

        auto &args = cmd.args.draw;
        args.first_vertex = first_vertex;
        args.vertex_count = vertex_count;

        commands.push(cmd);
    }

    void CommandBufferGl::draw_instanced(uint32_t vertex_count, uint32_t instance_count) {
        Command cmd;
        cmd.type = CommandType::DrawInstanced;

        auto &args = cmd.args.draw_instanced;
        args.vertex_count = vertex_count;
        args.instance_count = instance_count;

        commands.push(cmd);
    }

    void CommandBufferGl::end_render_pass() {

    }

    void CommandBufferGl::begin_compute_pass() {

    }

    void CommandBufferGl::dispatch(uint32_t group_size_x,
                                   uint32_t group_size_y,
                                   uint32_t group_size_z) {
        if (group_size_x == 0 || group_size_y == 0 || group_size_z == 0) {
            Logger::error("Compute group size cannot be zero!", "ComputeProgram");
            return;
        }

        Command cmd;
        cmd.type = CommandType::Dispatch;

        auto &args = cmd.args.dispatch;
        args.group_size_x = group_size_x;
        args.group_size_y = group_size_y;
        args.group_size_z = group_size_z;

        commands.push(cmd);
    }

    void CommandBufferGl::end_compute_pass() {

    }

    void CommandBufferGl::upload_to_buffer(const std::shared_ptr<Buffer> &buffer, uint32_t offset, uint32_t data_size,
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
    }

    void CommandBufferGl::upload_to_texture(const std::shared_ptr<Texture> &texture, Rect<uint32_t> p_region,
                                            const void *data) {
        // Invalid region represents the whole texture.
        auto region = p_region.is_valid() ? p_region : Rect<uint32_t>(0, 0, texture->get_width(),
                                                                      texture->get_height());

        Command cmd;
        cmd.type = CommandType::UploadToTexture;

        auto &args = cmd.args.upload_to_texture;
        args.texture = texture.get();
        args.offset_x = region.left;
        args.offset_y = region.top;
        args.width = region.width();
        args.height = region.height();
        args.data = data;

        commands.push(cmd);
    }

    void CommandBufferGl::read_buffer(const std::shared_ptr<Buffer> &buffer,
                                      uint32_t offset,
                                      uint32_t data_size,
                                      void *data) {
        switch (buffer->type) {
            case BufferType::Vertex:
            case BufferType::Uniform: {
                Logger::error("It's not possible to read data from vertex/uniform buffers!", "Command Buffer");
            }
                break;
            case BufferType::General: {
                Command cmd;
                cmd.type = CommandType::ReadBuffer;

                auto &args = cmd.args.read_buffer;
                args.buffer = buffer.get();
                args.offset = offset;
                args.data_size = data_size;
                args.data = data;

                commands.push(cmd);
            }
                break;
        }
    }

    void CommandBufferGl::submit(const std::shared_ptr<Driver> &p_driver) {
        while (!commands.empty()) {
            auto &cmd = commands.front();

            switch (cmd.type) {
                case CommandType::BeginRenderPass: {
                    auto &args = cmd.args.begin_render_pass;

                    auto framebuffer_gl = dynamic_cast<FramebufferGl *>(args.framebuffer);

                    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_gl->get_framebuffer_id());

                    if (args.clear) {
                        glClearColor(args.clear_color.r, args.clear_color.g, args.clear_color.b,
                                     args.clear_color.a);
                        glClear(GL_COLOR_BUFFER_BIT);
                    }

                    glViewport(0, 0, args.extent.x, args.extent.y);

                    check_error("BeginRenderPass");
                }
                    break;
                case CommandType::BindRenderPipeline: {
                    auto &args = cmd.args.bind_render_pipeline;

                    auto pipeline_gl = static_cast<RenderPipelineGl *>(args.pipeline);

                    auto blend_state = pipeline_gl->get_blend_state();

                    // Color blend.
                    glEnable(GL_BLEND);
                    glBlendFunc(to_gl_blend_factor(blend_state.src_blend_factor),
                                to_gl_blend_factor(blend_state.dst_blend_factor));

                    pipeline_gl->get_program()->use();

                    render_pipeline = args.pipeline;
                    compute_pipeline = nullptr;
                }
                    break;
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
                            case DataType::BYTE:
                            case DataType::UNSIGNED_BYTE:
                            case DataType::SHORT:
                            case DataType::UNSIGNED_SHORT:
                            case DataType::INT:
                            case DataType::UNSIGNED_INT: {
                                glVertexAttribIPointer(location,
                                                       attrib.size,
                                                       to_gl_data_type(attrib.type),
                                                       attrib.stride,
                                                       (void *) attrib.offset);
                            }
                                break;
                            case DataType::HALF_FLOAT:
                            case DataType::FLOAT: {
                                glVertexAttribPointer(location,
                                                      attrib.size,
                                                      to_gl_data_type(attrib.type),
                                                      GL_FALSE,
                                                      attrib.stride,
                                                      (void *) attrib.offset);
                            }
                                break;
                        }

                        glEnableVertexAttribArray(location);

                        if (attrib.vertex_input_rate == VertexInputRate::VERTEX) {
                            glVertexAttribDivisor(location, 0);
                        } else {
                            glVertexAttribDivisor(location, 1);
                        }
                    }

                    check_error("BindVertexBuffers");
                }
                    break;
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

                    for (auto &pair: args.descriptor_set->get_descriptors()) {
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
                            }
                                break;
                            case DescriptorType::Texture: {
                                auto texture_gl = static_cast<TextureGl *>(descriptor.texture.get());

                                if (!binding_name.empty()) {
                                    glUniform1i(glGetUniformLocation(program_id, binding_name.c_str()), binding_point);
                                }
                                glActiveTexture(GL_TEXTURE0 + binding_point);
                                glBindTexture(GL_TEXTURE_2D, texture_gl->get_texture_id());
                            }
                                break;
#ifdef PATHFINDER_USE_D3D11
                                case DescriptorType::GeneralBuffer: {
                                    auto buffer_gl = static_cast<BufferGl *>(descriptor.buffer.get());

                                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, buffer_gl->id);
                                }
                                    break;
                                case DescriptorType::Image: {
                                    auto texture_gl = static_cast<TextureGl *>(descriptor.texture.get());

                                    glBindImageTexture(binding_point, texture_gl->get_texture_id(), 0, GL_FALSE, 0,
                                                       GL_READ_WRITE, GL_RGBA8);
                                }
                                    break;
#endif
                            default:
                                break;
                        }
                    }

                    check_error("BindDescriptorSet");
                }
                    break;
                case CommandType::Draw: {
                    auto &args = cmd.args.draw;

                    glDrawArrays(GL_TRIANGLES, (GLsizei) args.first_vertex, (GLsizei) args.vertex_count);

                    check_error("Draw");
                }
                    break;
                case CommandType::DrawInstanced: {
                    auto &args = cmd.args.draw_instanced;

                    glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei) args.vertex_count,
                                          (GLsizei) args.instance_count);

                    check_error("DrawInstanced");
                }
                    break;
                case CommandType::EndRenderPass: {

                }
                    break;
                case CommandType::BeginComputePass: {

                }
                    break;
                case CommandType::BindComputePipeline: {
                    auto &args = cmd.args.bind_compute_pipeline;

                    auto pipeline_gl = static_cast<ComputePipelineGl *>(args.pipeline);

                    pipeline_gl->get_program()->use();

                    render_pipeline = nullptr;
                    compute_pipeline = args.pipeline;
                }
                    break;
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
                }
                    break;
                case CommandType::EndComputePass: {

                }
                    break;
                case CommandType::UploadToBuffer: {
                    auto &args = cmd.args.upload_to_buffer;

                    auto buffer_gl = static_cast<BufferGl *>(args.buffer);

                    int gl_buffer_type;

                    switch (args.buffer->type) {
                        case BufferType::Uniform: {
                            gl_buffer_type = GL_UNIFORM_BUFFER;
                        }
                            break;
                        case BufferType::Vertex: {
                            gl_buffer_type = GL_ARRAY_BUFFER;
                        }
                            break;
#ifdef PATHFINDER_USE_D3D11
                            case BufferType::General: {
                                gl_buffer_type = GL_SHADER_STORAGE_BUFFER;
                            }
                                break;
#endif
                    }

                    glBindBuffer(gl_buffer_type, buffer_gl->id);
                    glBufferSubData(gl_buffer_type, args.offset, args.data_size, args.data);
                    glBindBuffer(gl_buffer_type, 0); // Unbind.

                    check_error("UploadToBuffer");
                }
                    break;
                case CommandType::ReadBuffer: {
                    auto &args = cmd.args.read_buffer;

                    auto buffer_gl = static_cast<BufferGl *>(args.buffer);

#ifdef PATHFINDER_USE_D3D11
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
                }
                    break;
                case CommandType::UploadToTexture: {
                    auto &args = cmd.args.upload_to_texture;

                    auto texture_gl = static_cast<TextureGl *>(args.texture);

                    glBindTexture(GL_TEXTURE_2D, texture_gl->get_texture_id());
                    glTexSubImage2D(GL_TEXTURE_2D, 0,
                                    args.offset_x, args.offset_y, args.width, args.height,
                                    to_gl_pixel_data_format(PixelDataFormat::RGBA),
                                    to_gl_data_type(args.texture->get_pixel_type()),
                                    args.data);
                    glBindTexture(GL_TEXTURE_2D, 0);

                    check_error("UploadToTexture");
                }
                    break;
                case CommandType::Max:
                    break;
            }

            commands.pop();
        }
    }
}

#endif
