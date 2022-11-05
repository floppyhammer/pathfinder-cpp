#include "texture_rect.h"

#include <utility>

#include "../../src/common/global_macros.h"
#include "../../src/common/math/basic.h"
#include "../../src/common/math/mat4x4.h"
#include "../../src/gpu/platform.h"

#ifdef PATHFINDER_USE_VULKAN
    #include "../../src/shaders/generated/blit_frag_spv.h"
    #include "../../src/shaders/generated/blit_vert_spv.h"
#else
    #include "../../src/shaders/generated/blit_frag.h"
    #include "../../src/shaders/generated/blit_vert.h"
#endif

TextureRect::TextureRect(const std::shared_ptr<Pathfinder::Driver> &driver,
                         const std::shared_ptr<Pathfinder::RenderPass> &render_pass,
                         float width,
                         float height) {
    size.x = width;
    size.y = height;

    // Set up vertex data (and buffer(s)) and configure vertex attributes.
    float vertices[] = {// Positions, UVs.
                        0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0,

                        0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0};

    vertex_buffer = driver->create_buffer(Pathfinder::BufferType::Vertex,
                                          sizeof(vertices),
                                          Pathfinder::MemoryProperty::DeviceLocal,
                                          "TextureRect vertex buffer");
    uniform_buffer = driver->create_buffer(Pathfinder::BufferType::Uniform,
                                           16 * sizeof(float),
                                           Pathfinder::MemoryProperty::HostVisibleAndCoherent,
                                           "TextureRect uniform buffer");

    auto cmd_buffer = driver->create_command_buffer(true, "Upload TextureRect vertex buffer");
    cmd_buffer->upload_to_buffer(vertex_buffer, 0, sizeof(vertices), (void *)vertices);
    cmd_buffer->submit(std::shared_ptr<Pathfinder::Driver>(driver));

    // Pipeline.
    {
#ifdef PATHFINDER_USE_VULKAN
        const auto vert_source =
            std::vector<char>(std::begin(Pathfinder::blit_vert_spv), std::end(Pathfinder::blit_vert_spv));
        const auto frag_source =
            std::vector<char>(std::begin(Pathfinder::blit_frag_spv), std::end(Pathfinder::blit_frag_spv));
#else
        const auto vert_source = std::vector<char>(std::begin(Pathfinder::blit_vert), std::end(Pathfinder::blit_vert));
        const auto frag_source = std::vector<char>(std::begin(Pathfinder::blit_frag), std::end(Pathfinder::blit_frag));
#endif

        std::vector<Pathfinder::VertexInputAttributeDescription> attribute_descriptions;
        attribute_descriptions.reserve(3);

        uint32_t stride = 4 * sizeof(float);

        attribute_descriptions.push_back(
            {0, 2, Pathfinder::DataType::f32, stride, 0, Pathfinder::VertexInputRate::Vertex});

        attribute_descriptions.push_back(
            {0, 2, Pathfinder::DataType::f32, stride, 2 * sizeof(float), Pathfinder::VertexInputRate::Vertex});

        auto blend_state = Pathfinder::BlendState::from_over();

        descriptor_set = driver->create_descriptor_set();
        descriptor_set->add_or_update({
            Pathfinder::Descriptor::uniform(0, Pathfinder::ShaderStage::Vertex, "bUniform", uniform_buffer),
            Pathfinder::Descriptor::sampler(1, Pathfinder::ShaderStage::Fragment, "uTexture"),
        });

        pipeline = driver->create_render_pipeline(vert_source,
                                                  frag_source,
                                                  attribute_descriptions,
                                                  blend_state,
                                                  descriptor_set,
                                                  render_pass,
                                                  "TextureRect pipeline");
    }
}

void TextureRect::set_texture(const std::shared_ptr<Pathfinder::Texture> &_texture) {
    texture = _texture;

    descriptor_set->add_or_update({
        Pathfinder::Descriptor::sampler(1, Pathfinder::ShaderStage::Fragment, "uTexture", texture),
    });
}

void TextureRect::draw(const std::shared_ptr<Pathfinder::Driver> &driver,
                       const std::shared_ptr<Pathfinder::CommandBuffer> &cmd_buffer,
                       const Pathfinder::Vec2I &viewport_size) {
    // Get MVP matrix.
    // -------------------------------------------------
    // The actual application order of these matrices is reverse.
    auto model_mat = Pathfinder::Mat4x4<float>(1.0f);
    model_mat = model_mat.translate(
        Pathfinder::Vec3F(position.x / viewport_size.x * 2.0f, position.y / viewport_size.y * 2.0f, 0.0f));
    model_mat = model_mat.translate(Pathfinder::Vec3F(-1.0, -1.0, 0.0f));
    model_mat = model_mat.scale(Pathfinder::Vec3F(scale.x, scale.y, 1.0f));
    model_mat =
        model_mat.scale(Pathfinder::Vec3F(size.x / viewport_size.x * 2.0f, size.y / viewport_size.y * 2.0f, 1.0f));

    auto mvp_mat = model_mat;
    // -------------------------------------------------

    auto one_time_cmd_buffer = driver->create_command_buffer(true, "Upload TextureRect uniform buffer");
    one_time_cmd_buffer->upload_to_buffer(uniform_buffer, 0, 16 * sizeof(float), &mvp_mat);
    one_time_cmd_buffer->sync_descriptor_set(descriptor_set);
    one_time_cmd_buffer->submit(driver);

    cmd_buffer->bind_render_pipeline(pipeline);

    cmd_buffer->bind_vertex_buffers({vertex_buffer});

    cmd_buffer->bind_descriptor_set(descriptor_set);

    cmd_buffer->draw(0, 6);
}
