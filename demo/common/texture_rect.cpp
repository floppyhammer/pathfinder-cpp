#include "texture_rect.h"

#ifdef PATHFINDER_USE_VULKAN
    #include "../../pathfinder/shaders/generated/blit_frag_spv.h"
    #include "../../pathfinder/shaders/generated/blit_vert_spv.h"
#else
    #include "../../pathfinder/shaders/generated/blit_frag.h"
    #include "../../pathfinder/shaders/generated/blit_vert.h"
#endif

using namespace Pathfinder;

TextureRect::TextureRect(const std::shared_ptr<Device> &_device,
                         const std::shared_ptr<Queue> &_queue,
                         const std::shared_ptr<RenderPass> &render_pass) {
    device = _device;
    queue = _queue;

    // Set up vertex data (and buffer(s)) and configure vertex attributes.
    float vertices[] = {
        // Positions, UVs.
        0.0, 0.0, 0.0, 0.0, // 0
        1.0, 0.0, 1.0, 0.0, // 1
        1.0, 1.0, 1.0, 1.0, // 2
        0.0, 0.0, 0.0, 0.0, // 3
        1.0, 1.0, 1.0, 1.0, // 4
        0.0, 1.0, 0.0, 1.0  // 5
    };

    vertex_buffer = device->create_buffer({BufferType::Vertex, sizeof(vertices), MemoryProperty::DeviceLocal},
                                          "TextureRect vertex buffer");
    uniform_buffer =
        device->create_buffer({BufferType::Uniform, 16 * sizeof(float), MemoryProperty::HostVisibleAndCoherent},
                              "TextureRect uniform buffer");

    sampler = device->create_sampler(SamplerDescriptor{});

    auto encoder = device->create_command_encoder("Upload TextureRect vertex buffer");
    encoder->write_buffer(vertex_buffer, 0, sizeof(vertices), (void *)vertices);
    _queue->submit_and_wait(encoder);

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

        uint32_t stride = 4 * sizeof(float);

        attribute_descriptions.push_back({0, 2, DataType::f32, stride, 0, VertexInputRate::Vertex});

        attribute_descriptions.push_back({0, 2, DataType::f32, stride, 2 * sizeof(float), VertexInputRate::Vertex});

        auto blend_state = BlendState::from_over();

        descriptor_set = device->create_descriptor_set();
        descriptor_set->add_or_update({
            Descriptor::uniform(0, ShaderStage::Vertex, "bUniform", uniform_buffer),
            Descriptor::sampled(1, ShaderStage::Fragment, "uTexture"),
        });

        pipeline = device->create_render_pipeline(device->create_shader_module(vert_source, ShaderStage::Vertex),
                                                  device->create_shader_module(frag_source, ShaderStage::Fragment),
                                                  attribute_descriptions,
                                                  blend_state,
                                                  descriptor_set,
                                                  render_pass,
                                                  "TextureRect pipeline");
    }
}

void TextureRect::set_texture(const std::shared_ptr<Texture> &new_texture) {
    texture = new_texture;

    descriptor_set->add_or_update({
        Descriptor::sampled(1, ShaderStage::Fragment, "uTexture", texture, sampler),
    });

    size = texture->get_size().to_f32();
}

void TextureRect::draw(const std::shared_ptr<CommandEncoder> &encoder, const Vec2I &framebuffer_size) {
    // Get MVP matrix.
    // -------------------------------
    // The actual application order of these matrices is reversed.
    auto model_mat = Mat4(1.0f);
    model_mat = model_mat.translate(Vec3F(position / framebuffer_size.to_f32() * 2.0f, 0.0f));
    model_mat = model_mat.translate(Vec3F(-1.0, -1.0, 0.0f));
    model_mat = model_mat.scale(Vec3F(scale, 1.0f));
    model_mat = model_mat.scale(Vec3F(size / framebuffer_size.to_f32() * 2.0f, 1.0f));

    auto mvp_mat = model_mat;
    // -------------------------------

    encoder->write_buffer(uniform_buffer, 0, 16 * sizeof(float), &mvp_mat);

    encoder->bind_render_pipeline(pipeline);

    encoder->bind_vertex_buffers({vertex_buffer});

    encoder->bind_descriptor_set(descriptor_set);

    encoder->draw(0, 6);
}
