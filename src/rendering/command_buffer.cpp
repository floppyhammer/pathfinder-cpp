//
// Created by floppyhammer on 4/9/2022.
//

#include "command_buffer.h"

#include "device.h"

namespace Pathfinder {
    void CommandBuffer::begin_render_pass(uint32_t framebuffer_id,
                                          Vec2<uint32_t> extent,
                                          bool clear,
                                          ColorF clear_color) {
        Command cmd;
        auto &args = cmd.args.begin_render_pass;

        args.framebuffer_id = framebuffer_id;
        args.extent = extent;
        args.clear = clear;
        args.clear_color = clear_color;
    }

    void CommandBuffer::end_render_pass() {

    }

    void CommandBuffer::bind_render_pipeline(std::shared_ptr<RenderPipeline> pipeline) {
        Command cmd;
        auto &args = cmd.args.bind_render_pipeline;

        args.pipeline = pipeline.get();
    }

    void CommandBuffer::bind_compute_pipeline() {

    }

    void CommandBuffer::bind_vertex_buffers() {

    }

    void CommandBuffer::bind_index_buffer() {

    }

    void CommandBuffer::bind_descriptor_set(const std::shared_ptr<DescriptorSet>& descriptor_set) {
        Command cmd;

        auto &args = cmd.args.bind_descriptor_set;
        args.descriptor_set = descriptor_set.get();

        commands.push(cmd);
    }

    void CommandBuffer::draw_instanced(uint32_t vertex_count, uint32_t instance_count) {

    }

    void CommandBuffer::dispatch(uint32_t group_size_x = 1,
                                 uint32_t group_size_y = 1,
                                 uint32_t group_size_z = 1) {
        if (group_size_x == 0 || group_size_y == 0 || group_size_z == 0) {
            Logger::error("Compute group size cannot be zero!", "ComputeProgram");
            return;
        }

        Command cmd;
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
                }
                    break;
                case CommandType::BindVertexBuffers: {
                    auto &args = cmd.args.bind_vertex_buffers;

                    glBindVertexArray(args.buffers->args.vertex.vao);

                    auto &attribute_descriptors = render_pipeline->attribute_descriptors;

                    for (int i = 0; i < attribute_descriptors.size(); i++) {
                        auto &attrib = attribute_descriptors[i];

                        if (i == 0) {
                            glBindBuffer(GL_ARRAY_BUFFER, attrib.vbo);
                        } else {
                            // If target VBO changed.
                            if (attrib.vbo != attribute_descriptors[i - 1].vbo) {
                                glBindBuffer(GL_ARRAY_BUFFER, attrib.vbo);
                            }
                        }

                        glVertexAttribIPointer(i,
                                               attrib.size,
                                               static_cast<GLenum>(attrib.type),
                                               attrib.stride,
                                               (void *) attrib.offset);

                        glEnableVertexAttribArray(i);

                        if (attrib.vertex_step == VertexStep::PER_VERTEX) {
                            glVertexAttribDivisor(i, 0);
                        } else {
                            glVertexAttribDivisor(i, 1);
                        }
                    }
                }
                    break;
                case CommandType::BindDescriptorSet: {
                    auto &args = cmd.args.bind_descriptor_set;

                    for (auto &pair: args.descriptor_set->descriptors) {
                        auto binding_point = pair.first;
                        auto &descriptor = pair.second;

                        auto binding_name = descriptor.binding_name;

                        switch (descriptor.type) {
                            case DescriptorType::UniformBuffer: {

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
