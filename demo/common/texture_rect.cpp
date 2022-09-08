#include "texture_rect.h"

#include <utility>

#include "../../src/common/math/basic.h"
#include "../../src/common/math/mat4x4.h"
#include "../../src/common/global_macros.h"
#include "../../src/gpu/platform.h"

TextureRect::TextureRect(const std::shared_ptr<Pathfinder::Driver> &driver,
                         const std::shared_ptr<Pathfinder::RenderPass> &render_pass,
                         uint32_t width,
                         uint32_t height) {
    size.x = width;
    size.y = height;

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

    vertex_buffer = driver->create_buffer(Pathfinder::BufferType::Vertex, sizeof(vertices),
                                          Pathfinder::MemoryProperty::DEVICE_LOCAL);
    uniform_buffer = driver->create_buffer(Pathfinder::BufferType::Uniform, 16 * sizeof(float),
                                           Pathfinder::MemoryProperty::HOST_VISIBLE_AND_COHERENT);

    auto cmd_buffer = driver->create_command_buffer(true);
    cmd_buffer->upload_to_buffer(vertex_buffer,
                                 0,
                                 sizeof(vertices),
                                 (void *) vertices);
    cmd_buffer->submit(std::shared_ptr<Pathfinder::Driver>(driver));

    // Pipeline.
    {
        std::string postfix;
#ifdef PATHFINDER_USE_VULKAN
        postfix = ".spv";
#endif

        const auto vert_source = Pathfinder::load_file_as_bytes(PATHFINDER_SHADER_DIR"blit.vert" + postfix);
        const auto frag_source = Pathfinder::load_file_as_bytes(PATHFINDER_SHADER_DIR"blit.frag" + postfix);

        std::vector<Pathfinder::VertexInputAttributeDescription> attribute_descriptions;
        attribute_descriptions.reserve(3);

        uint32_t stride = 8 * sizeof(float);

        attribute_descriptions.push_back({0,
                                          3,
                                          Pathfinder::DataType::FLOAT,
                                          stride,
                                          0,
                                          Pathfinder::VertexInputRate::VERTEX});

        attribute_descriptions.push_back({0,
                                          3,
                                          Pathfinder::DataType::FLOAT,
                                          stride,
                                          3 * sizeof(float),
                                          Pathfinder::VertexInputRate::VERTEX});

        attribute_descriptions.push_back({0,
                                          2,
                                          Pathfinder::DataType::FLOAT,
                                          stride,
                                          6 * sizeof(float),
                                          Pathfinder::VertexInputRate::VERTEX});

        Pathfinder::ColorBlendState blend_state = {true, Pathfinder::BlendFactor::ONE,
                                                   Pathfinder::BlendFactor::ONE_MINUS_SRC_ALPHA};

        {
            descriptor_set = driver->create_descriptor_set();

            descriptor_set->add_or_update_descriptor({
                                                             Pathfinder::DescriptorType::UniformBuffer,
                                                             Pathfinder::ShaderType::Vertex,
                                                             0,
                                                             "bUniform",
                                                             uniform_buffer, nullptr});
            descriptor_set->add_or_update_descriptor({
                                                             Pathfinder::DescriptorType::Texture,
                                                             Pathfinder::ShaderType::Fragment,
                                                             1,
                                                             "uTexture",
                                                             nullptr, nullptr});
        }

        pipeline = driver->create_render_pipeline({vert_source.begin(), vert_source.end()},
                                                  {frag_source.begin(), frag_source.end()},
                                                  attribute_descriptions,
                                                  blend_state,
                                                  descriptor_set,
                                                  render_pass);
    }
}

void TextureRect::set_texture(std::shared_ptr<Pathfinder::Texture> p_texture) {
    texture = std::move(p_texture);

    Pathfinder::Descriptor descriptor;
    descriptor.type = Pathfinder::DescriptorType::Texture;
    descriptor.stage = Pathfinder::ShaderType::Fragment;
    descriptor.binding = 1;
    descriptor.binding_name = "uTexture";
    descriptor.texture = texture;

    descriptor_set->add_or_update_descriptor(descriptor);
}

void TextureRect::draw(const std::shared_ptr<Pathfinder::Driver> &driver,
                       const std::shared_ptr<Pathfinder::CommandBuffer> &cmd_buffer,
                       const Pathfinder::Vec2<uint32_t> &viewport_size) {
    // Get MVP matrix.
    // -------------------------------------------------
    // The actual application order of these matrices is reverse.
    auto model_mat = Pathfinder::Mat4x4<float>(1.0f);
    model_mat = model_mat.translate(Pathfinder::Vec3<float>(position.x / viewport_size.x * 2.0f,
                                                            position.y / viewport_size.y * 2.0f,
                                                            0.0f));
    model_mat = model_mat.translate(Pathfinder::Vec3<float>(-1.0, -1.0, 0.0f));
    model_mat = model_mat.scale(Pathfinder::Vec3<float>(scale.x, scale.y, 1.0f));
    model_mat = model_mat.scale(Pathfinder::Vec3<float>(size.x / viewport_size.x * 2.0f,
                                                        size.y / viewport_size.y * 2.0f,
                                                        1.0f));

    auto mvp_mat = model_mat;
    // -------------------------------------------------

    auto one_time_cmd_buffer = driver->create_command_buffer(true);
    one_time_cmd_buffer->upload_to_buffer(uniform_buffer, 0, 16 * sizeof(float), &mvp_mat);
    one_time_cmd_buffer->submit(driver);

    cmd_buffer->bind_render_pipeline(pipeline);

    cmd_buffer->bind_vertex_buffers({vertex_buffer});

    cmd_buffer->bind_descriptor_set(descriptor_set);

    cmd_buffer->draw(0, 6);
}
