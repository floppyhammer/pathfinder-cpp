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

TextureRect::TextureRect(const std::shared_ptr<Driver> &_driver,
                         const std::shared_ptr<RenderPass> &render_pass,
                         const Vec2F &_size) {
    driver = _driver;
    size = _size;

    // Set up vertex data (and buffer(s)) and configure vertex attributes.
    float vertices[] = {// Positions, UVs.
                        0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0,

                        0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0};

    vertex_buffer = driver->create_buffer(BufferType::Vertex,
                                          sizeof(vertices),
                                          MemoryProperty::DeviceLocal,
                                          "TextureRect vertex buffer");
    uniform_buffer = driver->create_buffer(BufferType::Uniform,
                                           16 * sizeof(float),
                                           MemoryProperty::HostVisibleAndCoherent,
                                           "TextureRect uniform buffer");

    auto cmd_buffer = driver->create_command_buffer("Upload TextureRect vertex buffer");
    cmd_buffer->upload_to_buffer(vertex_buffer, 0, sizeof(vertices), (void *)vertices);
    cmd_buffer->submit_and_wait();

    // Pipeline.
    {
#ifdef PATHFINDER_USE_VULKAN
        const auto vert_source = std::vector<char>(std::begin(blit_vert_spv), std::end(blit_vert_spv));
        const auto frag_source = std::vector<char>(std::begin(blit_frag_spv), std::end(blit_frag_spv));
#else
        const auto vert_source = std::vector<char>(std::begin(blit_vert), std::end(blit_vert));
        const auto frag_source = std::vector<char>(std::begin(blit_frag), std::end(blit_frag));
#endif

        std::vector<VertexInputAttributeDescription> attribute_descriptions;
        attribute_descriptions.reserve(3);

        uint32_t stride = 4 * sizeof(float);

        attribute_descriptions.push_back({0, 2, DataType::f32, stride, 0, VertexInputRate::Vertex});

        attribute_descriptions.push_back({0, 2, DataType::f32, stride, 2 * sizeof(float), VertexInputRate::Vertex});

        auto blend_state = BlendState::from_over();

        descriptor_set = driver->create_descriptor_set();
        descriptor_set->add_or_update({
            Descriptor::uniform(0, ShaderStage::Vertex, "bUniform", uniform_buffer),
            Descriptor::sampler(1, ShaderStage::Fragment, "uTexture"),
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

void TextureRect::set_texture(const std::shared_ptr<Texture> &_texture) {
    texture = _texture;

    descriptor_set->add_or_update({
        Descriptor::sampler(1, ShaderStage::Fragment, "uTexture", texture),
    });
}

void TextureRect::draw(const std::shared_ptr<CommandBuffer> &cmd_buffer, const Vec2I &viewport_size) {
    // Get MVP matrix.
    // -------------------------------------------------
    // The actual application order of these matrices is reverse.
    auto model_mat = Mat4x4<float>(1.0f);
    model_mat =
        model_mat.translate(Vec3F(position.x / viewport_size.x * 2.0f, position.y / viewport_size.y * 2.0f, 0.0f));
    model_mat = model_mat.translate(Vec3F(-1.0, -1.0, 0.0f));
    model_mat = model_mat.scale(Vec3F(scale.x, scale.y, 1.0f));
    model_mat = model_mat.scale(Vec3F(size.x / viewport_size.x * 2.0f, size.y / viewport_size.y * 2.0f, 1.0f));

    auto mvp_mat = model_mat;
    // -------------------------------------------------

    auto one_time_cmd_buffer = driver->create_command_buffer("Upload TextureRect uniform buffer");
    one_time_cmd_buffer->upload_to_buffer(uniform_buffer, 0, 16 * sizeof(float), &mvp_mat);
    one_time_cmd_buffer->sync_descriptor_set(descriptor_set);
    one_time_cmd_buffer->submit_and_wait();

    cmd_buffer->bind_render_pipeline(pipeline);

    cmd_buffer->bind_vertex_buffers({vertex_buffer});

    cmd_buffer->bind_descriptor_set(descriptor_set);

    cmd_buffer->draw(0, 6);
}
