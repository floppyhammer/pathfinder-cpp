#include "device.h"

#include "render_pipeline.h"
#include "compute_pipeline.h"
#include "validation.h"

#include "../../common/logger.h"

namespace Pathfinder {
    std::shared_ptr<Framebuffer> DeviceGl::create_framebuffer(uint32_t p_width, uint32_t p_height,
                                                              TextureFormat p_format, DataType p_type) {
        auto framebuffer = std::make_shared<FramebufferGl>(p_width, p_height, p_format, p_type);

        check_error("create_framebuffer");
        return framebuffer;
    }

    std::shared_ptr<Buffer> DeviceGl::create_buffer(BufferType type, size_t size) {
        if (size == 0) {
            Logger::error("Tried to create a buffer with zero size!");
        }

        auto buffer = std::make_shared<BufferGl>();

        switch (type) {
            case BufferType::Uniform: {
                unsigned int buffer_id;
                glGenBuffers(1, &buffer_id);
                glBindBuffer(GL_UNIFORM_BUFFER, buffer_id);
                glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

                buffer->type = BufferType::Uniform;
                buffer->size = size;
                buffer->id = buffer_id;
            }
                break;
            case BufferType::Vertex: {
                unsigned int buffer_id;
                glGenBuffers(1, &buffer_id);
                glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
                glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

                buffer->type = BufferType::Vertex;
                buffer->size = size;
                buffer->id = buffer_id;
            }
                break;
                case BufferType::General: {
#ifdef PATHFINDER_USE_D3D11
                    if (size < MAX_BUFFER_SIZE_CLASS) {
                        size = upper_power_of_two(size);
                    }

                    GLuint buffer_id;
                    glGenBuffers(1, &buffer_id);

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind.

                    buffer->type = BufferType::General;
                    buffer->size = size;
                    buffer->id = buffer_id;
#endif
                }
                    break;
        }

        check_error("create_buffer");
        return buffer;
    }

    std::shared_ptr<Texture> DeviceGl::create_texture(uint32_t p_width,
                                                      uint32_t p_height,
                                                      TextureFormat p_format,
                                                      DataType p_type) {
        auto texture = std::make_shared<TextureGl>(p_width, p_height, p_format, p_type);

        check_error("create_texture");
        return texture;
    }

    std::shared_ptr<CommandBuffer> DeviceGl::create_command_buffer() {
        check_error("create_command_buffer");
        return std::make_shared<CommandBufferGl>();
    }

    std::shared_ptr<RenderPipeline> DeviceGl::create_render_pipeline(const std::string &vert_source,
                                                                     const std::string &frag_source,
                                                                     const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                                                                     ColorBlendState blend_state) {
        auto pipeline_gl = std::make_shared<RenderPipelineGl>(vert_source,
                                                           frag_source,
                                                           attribute_descriptions,
                                                           blend_state);

        return pipeline_gl;
    }

    std::shared_ptr<ComputePipeline> DeviceGl::create_compute_pipeline(const std::string &comp_source) {
        return std::make_shared<ComputePipelineGl>(comp_source);
    }
}
