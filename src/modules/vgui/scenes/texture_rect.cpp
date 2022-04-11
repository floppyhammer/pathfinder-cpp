//
// Created by floppyhammer on 6/7/2021.
//

#include "texture_rect.h"

#include "../../../common/math/basic.h"
#include "../../../common/math/mat4x4.h"
#include "../../../common/global_macros.h"
#include "../../../rendering/command_buffer.h"

namespace Pathfinder {
    TextureRect::TextureRect(float viewport_width, float viewport_height) {
        rect_size.x = viewport_width;
        rect_size.y = viewport_height;

        // Set VAO&VBO.
        // ---------------------------
        // Set up vertex data (and buffer(s)) and configure vertex attributes.
        float vertices[] = {
                // Positions, colors, UVs.
                0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
                1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
                1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0,

                0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
                1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0,
                0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0
        };

        vertex_buffer = Device::create_buffer(BufferType::Vertex, sizeof(vertices));
        Device::upload_to_buffer(vertex_buffer,
                                 0,
                                 sizeof(vertices),
                                 (void *) vertices);

        uniform_buffer = Device::create_buffer(BufferType::Uniform, 16 * sizeof(float));

        // Pipeline.
        {
            const std::string vert_source =
#include "../src/shaders/blit.vert"
            ;
            const std::string frag_source =
#include "../src/shaders/blit.frag"
            ;

            pipeline = std::make_shared<RenderPipeline>();
            pipeline->program = std::make_shared<RasterProgram>(vert_source, frag_source);

            pipeline->blend_src = GL_ONE;
            pipeline->blend_dst = GL_ONE_MINUS_SRC_ALPHA;

            auto &attribute_descriptors = pipeline->attribute_descriptors;
            attribute_descriptors.reserve(3);

            uint32_t stride = 8 * sizeof(float);

            attribute_descriptors.push_back({0,
                                             3,
                                             DataType::FLOAT,
                                             stride,
                                             0,
                                             VertexStep::PER_VERTEX});

            attribute_descriptors.push_back({0,
                                             3,
                                             DataType::FLOAT,
                                             stride,
                                             3 * sizeof(float),
                                             VertexStep::PER_VERTEX});

            attribute_descriptors.push_back({0,
                                             2,
                                             DataType::FLOAT,
                                             stride,
                                             6 * sizeof(float),
                                             VertexStep::PER_VERTEX});
        }

        {
            descriptor_set = std::make_shared<DescriptorSet>();

            Descriptor descriptor;
            descriptor.type = DescriptorType::UniformBuffer;
            descriptor.binding = 0;
            descriptor.binding_name = "bUniform";
            descriptor.buffer = uniform_buffer;

            descriptor_set->add_descriptor(descriptor);
        }
    }

    void TextureRect::set_texture(std::shared_ptr<Texture> p_texture) {
        texture = std::move(p_texture);

        Descriptor descriptor;
        descriptor.type = DescriptorType::Texture;
        descriptor.binding = 0;
        descriptor.binding_name = "uTexture";
        descriptor.texture = texture;

        descriptor_set->add_descriptor(descriptor);
    }

    std::shared_ptr<Texture> TextureRect::get_texture() const {
        return texture;
    }

    void TextureRect::draw(const std::shared_ptr<Pathfinder::CommandBuffer>& cmd_buffer,
                           const std::shared_ptr<Viewport>& render_target) {
        // Get MVP matrix.
        // -------------------------------------------------
        // The actual application order of these matrices is reverse.
        auto model_mat = Mat4x4<float>(1.0f);
        model_mat = model_mat.translate(Vec3<float>(rect_position.x / render_target->get_width() * 2.0f,
                                                    rect_position.y / render_target->get_height() * 2.0f,
                                                    0.0f));
        model_mat = model_mat.translate(Vec3<float>(-1.0, -1.0, 0.0f));
        model_mat = model_mat.scale(Vec3<float>(rect_scale.x, rect_scale.y, 1.0f));
        model_mat = model_mat.scale(Vec3<float>(rect_size.x / render_target->get_width() * 2.0f,
                                                rect_size.y / render_target->get_height() * 2.0f,
                                                1.0f));
        //model_mat = model_mat.rotate(deg2rad(rect_rotation), Vec3<float>(0.0f, 0.0f, 1.0f));

        auto mvp_mat = model_mat;
        // -------------------------------------------------

        Device::upload_to_buffer(uniform_buffer, 0, 16 * sizeof(float), &mvp_mat);

        cmd_buffer->bind_render_pipeline(pipeline);

        cmd_buffer->bind_vertex_buffers({vertex_buffer});

        cmd_buffer->bind_descriptor_set(descriptor_set);

        cmd_buffer->draw(0, 6);
    }
}
