//
// Created by floppyhammer on 4/9/2022.
//

#include "command_buffer.h"

#include "device.h"

#include <cassert>

namespace Pathfinder {
    void CommandBuffer::begin_render_pass(uint32_t framebuffer_id,
                                          Vec2<uint32_t> extent,
                                          bool clear,
                                          ColorF clear_color) {
        Command cmd;
        cmd.type = CommandType::BeginRenderPass;

        auto &args = cmd.args.begin_render_pass;
        args.framebuffer_id = framebuffer_id;
        args.extent = extent;
        args.clear = clear;
        args.clear_color = clear_color;

        commands.push(cmd);
    }

    void CommandBuffer::end_render_pass() {

    }

    void CommandBuffer::bind_render_pipeline(const std::shared_ptr<RenderPipeline>& pipeline) {
        Command cmd;
        cmd.type = CommandType::BindRenderPipeline;
        auto &args = cmd.args.bind_render_pipeline;
        args.pipeline = pipeline.get();

        commands.push(cmd);
    }

    void CommandBuffer::bind_compute_pipeline() {

    }

    void CommandBuffer::bind_vertex_buffers(std::vector<std::shared_ptr<Buffer>> vertex_buffers) {
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

    void CommandBuffer::bind_descriptor_set(const std::shared_ptr<DescriptorSet>& descriptor_set) {
        Command cmd;
        cmd.type = CommandType::BindDescriptorSet;

        auto &args = cmd.args.bind_descriptor_set;
        args.descriptor_set = descriptor_set.get();

        commands.push(cmd);
    }

    void CommandBuffer::draw(uint32_t first_vertex, uint32_t vertex_count) {
        Command cmd;
        cmd.type = CommandType::Draw;

        auto &args = cmd.args.draw;
        args.first_vertex = first_vertex;
        args.vertex_count = vertex_count;

        commands.push(cmd);
    }

    void CommandBuffer::draw_instanced(uint32_t vertex_count, uint32_t instance_count) {
        Command cmd;
        cmd.type = CommandType::DrawInstanced;

        auto &args = cmd.args.draw_instanced;
        args.vertex_count = vertex_count;
        args.instance_count = instance_count;

        commands.push(cmd);
    }

    void CommandBuffer::dispatch(uint32_t group_size_x = 1,
                                 uint32_t group_size_y = 1,
                                 uint32_t group_size_z = 1) {
        if (group_size_x == 0 || group_size_y == 0 || group_size_z == 0) {
            Logger::error("Compute group size cannot be zero!", "ComputeProgram");
            return;
        }

        Command cmd;
        cmd.type = CommandType::DrawInstanced;

        auto &args = cmd.args.dispatch;
        args.group_size_x = group_size_x;
        args.group_size_y = group_size_y;
        args.group_size_z = group_size_z;

        commands.push(cmd);
    }

    void CommandBuffer::end_compute_pass() {

    }

    // Data

    void CommandBuffer::submit() {
        while (!commands.empty()) {
            auto &cmd = commands.front();

            switch (cmd.type) {
                case CommandType::BeginRenderPass: {
                    auto &args = cmd.args.begin_render_pass;

                    glBindFramebuffer(GL_FRAMEBUFFER, args.framebuffer_id);

                    if (args.clear) {
                        glClearColor(args.clear_color.r, args.clear_color.g, args.clear_color.b, args.clear_color.a);
                        glClear(GL_COLOR_BUFFER_BIT);
                    }

                    glViewport(0, 0, args.extent.x, args.extent.y);

                    Device::check_error("BeginRenderPass");
                }
                    break;
                case CommandType::BindRenderPipeline: {
                    auto &args = cmd.args.bind_render_pipeline;

                    // Color blend.
                    glEnable(GL_BLEND);
                    glBlendFunc(args.pipeline->blend_src, args.pipeline->blend_dst);

                    args.pipeline->program->use();

                    render_pipeline = args.pipeline;
                }
                    break;
                case CommandType::BindVertexBuffers: {
                    auto &args = cmd.args.bind_vertex_buffers;

                    auto buffer_count = args.buffer_count;
                    auto vertex_buffers = args.buffers;

                    glBindVertexArray(render_pipeline->vao);

                    auto &attribute_descriptors = render_pipeline->attribute_descriptors;

                    auto last_vbo = 0;
                    for (int location = 0; location < attribute_descriptors.size(); location++) {
                        auto &attrib = attribute_descriptors[location];

                        if (attrib.binding >= buffer_count) {
                            assert("Vertex buffer binding exceeded buffer count!");
                            return;
                        }

                        auto buffer = vertex_buffers[attrib.binding];
                        auto vbo = buffer->args.vertex.vbo;

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
                                                       static_cast<GLenum>(attrib.type),
                                                       attrib.stride,
                                                       (void *) attrib.offset);
                            } break;
                            case DataType::HALF_FLOAT:
                            case DataType::FLOAT: {
                                glVertexAttribPointer(location,
                                                      attrib.size,
                                                      static_cast<GLenum>(attrib.type),
                                                      GL_FALSE,
                                                      attrib.stride,
                                                      (void *) attrib.offset);
                            } break;
                        }

                        glEnableVertexAttribArray(location);

                        if (attrib.vertex_step == VertexStep::PER_VERTEX) {
                            glVertexAttribDivisor(location, 0);
                        } else {
                            glVertexAttribDivisor(location, 1);
                        }
                    }

                    Device::check_error("BindVertexBuffers");
                }
                    break;
                case CommandType::BindDescriptorSet: {
                    auto &args = cmd.args.bind_descriptor_set;

                    for (auto &pair: args.descriptor_set->get_descriptors()) {
                        auto &descriptor = pair.second;

                        // Note that pair.first is not the binding point.
                        auto binding_point = descriptor.binding;
                        auto binding_name = descriptor.binding_name;

                        switch (descriptor.type) {
                            case DescriptorType::UniformBuffer: {
                                auto buffer = descriptor.buffer.value();

                                unsigned int ubo_index = glGetUniformBlockIndex(render_pipeline->program->get_id(), binding_name.c_str());
                                glUniformBlockBinding(render_pipeline->program->get_id(), ubo_index, binding_point);
                                glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, buffer->args.uniform.ubo);
                            }
                                break;
                            case DescriptorType::Texture: {
                                auto texture = descriptor.texture.value();

                                if (!binding_name.empty()) {
                                    glUniform1i(glGetUniformLocation(render_pipeline->program->get_id(), binding_name.c_str()),
                                                binding_point);
                                }
                                glActiveTexture(GL_TEXTURE0 + binding_point);
                                glBindTexture(GL_TEXTURE_2D, texture->get_texture_id());
                            }
                                break;
                            case DescriptorType::Image: {

                            }
                                break;
                            default:
                                break;
                        }
                    }

                    Device::check_error("BindDescriptorSet");
                }
                    break;
                case CommandType::Draw: {
                    auto &args = cmd.args.draw;

                    glDrawArrays(GL_TRIANGLES, (GLsizei) args.first_vertex, (GLsizei) args.vertex_count);

                    Device::check_error("Draw");
                }
                    break;
                case CommandType::DrawInstanced: {
                    auto &args = cmd.args.draw_instanced;

                    glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei) args.vertex_count, (GLsizei) args.instance_count);

                    Device::check_error("DrawInstanced");
                }
                    break;
                case CommandType::EndRenderPass: {

                }
                    break;
                case CommandType::BeginComputePass: {

                }
                    break;
                case CommandType::BindComputePipeline: {

                }
                    break;
                case CommandType::Dispatch: {
                    auto &args = cmd.args.dispatch;

                    // Max global (total) work group counts x:2147483647 y:65535 z:65535.
                    // Max local (in one shader) work group sizes x:1536 y:1024 z:64.
                    glDispatchCompute(args.group_size_x, args.group_size_y, args.group_size_z);

                    // In order to use timestamps more precisely.
#ifdef PATHFINDER_DEBUG
                    glFinish();
#endif

                    Device::check_error("Dispatch");
                }
                    break;
                case CommandType::EndComputePass: {

                }
                    break;
                case CommandType::UploadGeneralBuffer: {

                }
                    break;
                case CommandType::ReadGeneralBuffer: {

                }
                    break;
                case CommandType::Max:
                    break;
            }

            commands.pop();
        }
    }
}
