#include "texture_rect.h"

#include "../../../common/math/basic.h"
#include "../../../common/math/mat4x4.h"
#include "../../../common/global_macros.h"
#include "../../../gpu/platform.h"

namespace Pathfinder {
    TextureRect::TextureRect(const std::shared_ptr<Driver> &driver, float viewport_width, float viewport_height) {
        type = NodeType::TextureRect;

        rect_size.x = viewport_width;
        rect_size.y = viewport_height;

        // Set VAO&VBO.
        // ---------------------------
        // Set up vertex data (and buffer(s)) and configure vertex attributes.
        float vertices[] = {
                // Positions, colors, UVs.
                0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
                1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0,
                1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0,

                0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
                1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0,
                0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0
        };

        vertex_buffer = driver->create_buffer(BufferType::Vertex, sizeof(vertices));
        uniform_buffer = driver->create_buffer(BufferType::Uniform, 16 * sizeof(float));

        auto cmd_buffer = driver->create_command_buffer();
        cmd_buffer->upload_to_buffer(vertex_buffer,
                                     0,
                                     sizeof(vertices),
                                     (void *) vertices);
        cmd_buffer->submit();

        // Pipeline.
        {
            const std::string vert_source =
#include "../src/shaders/blit.vert"
        ;
            const std::string frag_source =
#include "../src/shaders/blit.frag"
        ;

            std::vector<VertexInputAttributeDescription> attribute_descriptions;
            attribute_descriptions.reserve(3);

            uint32_t stride = 8 * sizeof(float);

            attribute_descriptions.push_back({0,
                                              3,
                                              DataType::FLOAT,
                                              stride,
                                              0,
                                              VertexInputRate::VERTEX});

            attribute_descriptions.push_back({0,
                                              3,
                                              DataType::FLOAT,
                                              stride,
                                              3 * sizeof(float),
                                              VertexInputRate::VERTEX});

            attribute_descriptions.push_back({0,
                                              2,
                                              DataType::FLOAT,
                                              stride,
                                              6 * sizeof(float),
                                              VertexInputRate::VERTEX});

            ColorBlendState blend_state = {true, BlendFactor::ONE, BlendFactor::ONE_MINUS_SRC_ALPHA};

            // FIXME
            auto render_pass = driver->create_render_pass();

            pipeline = driver->create_render_pipeline({vert_source.begin(), vert_source.end()},
                                                      {frag_source.begin(), frag_source.end()},
                                                      attribute_descriptions,
                                                      blend_state,
                                                      render_pass);
        }

        {
            descriptor_set = std::make_shared<DescriptorSet>();

            descriptor_set->add_or_update_descriptor({
                                                             DescriptorType::UniformBuffer,
                                                             0,
                                                             "bUniform",
                                                             uniform_buffer});
        }
    }

    void TextureRect::set_texture(std::shared_ptr<Texture> p_texture) {
        texture = std::move(p_texture);

        Descriptor descriptor;
        descriptor.type = DescriptorType::Texture;
        descriptor.binding = 0;
        descriptor.binding_name = "uTexture";
        descriptor.texture = texture;

        descriptor_set->add_or_update_descriptor(descriptor);
    }

    std::shared_ptr<Texture> TextureRect::get_texture() const {
        return texture;
    }

    void TextureRect::draw(const std::shared_ptr<Driver> &driver,
                           const std::shared_ptr<Pathfinder::CommandBuffer> &cmd_buffer,
                           const Vec2<uint32_t> &framebuffer_size) {
        // Get MVP matrix.
        // -------------------------------------------------
        // The actual application order of these matrices is reverse.
        auto model_mat = Mat4x4<float>(1.0f);
        model_mat = model_mat.translate(Vec3<float>(rect_position.x / framebuffer_size.x * 2.0f,
                                                    rect_position.y / framebuffer_size.y * 2.0f,
                                                    0.0f));
        model_mat = model_mat.translate(Vec3<float>(-1.0, -1.0, 0.0f));
        model_mat = model_mat.scale(Vec3<float>(rect_scale.x, rect_scale.y, 1.0f));
        model_mat = model_mat.scale(Vec3<float>(rect_size.x / framebuffer_size.x * 2.0f,
                                                rect_size.y / framebuffer_size.y * 2.0f,
                                                1.0f));
        //model_mat = model_mat.rotate(deg2rad(rect_rotation), Vec3<float>(0.0f, 0.0f, 1.0f));

        auto mvp_mat = model_mat;
        // -------------------------------------------------

        auto one_shot_cmd_buffer = driver->create_command_buffer();
        one_shot_cmd_buffer->upload_to_buffer(uniform_buffer, 0, 16 * sizeof(float), &mvp_mat);
        one_shot_cmd_buffer->submit();

        cmd_buffer->bind_render_pipeline(pipeline);

        cmd_buffer->bind_vertex_buffers({vertex_buffer});

        cmd_buffer->bind_descriptor_set(descriptor_set);

        cmd_buffer->draw(0, 6);
    }
}
