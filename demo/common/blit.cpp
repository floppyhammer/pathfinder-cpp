#include "blit.h"

#ifdef PATHFINDER_USE_VULKAN
    #include "../../pathfinder/shaders/generated/blit_frag_spv.h"
    #include "../../pathfinder/shaders/generated/blit_vert_spv.h"
#else
    #include "../../pathfinder/shaders/generated/blit_frag.h"
    #include "../../pathfinder/shaders/generated/blit_vert.h"
#endif

using namespace Pathfinder;

Blit::Blit(const std::shared_ptr<Device> &_device,
           const std::shared_ptr<Queue> &_queue,
           const std::shared_ptr<RenderPass> &render_pass) {
    device = _device;
    queue = _queue;

    // Set up vertex data (and buffer(s)) and configure vertex attributes.
    float vertices[] = {
        // Positions, UVs.
        -1.0, -1.0, 0.0, 0.0, // 0
        1.0,  -1.0, 1.0, 0.0, // 1
        1.0,  1.0,  1.0, 1.0, // 2
        -1.0, -1.0, 0.0, 0.0, // 3
        1.0,  1.0,  1.0, 1.0, // 4
        -1.0, 1.0,  0.0, 1.0  // 5
    };

    vertex_buffer = device->create_buffer({BufferType::Vertex, sizeof(vertices), MemoryProperty::DeviceLocal},
                                          "blit vertex buffer");

    sampler = device->create_sampler(SamplerDescriptor{});

    auto encoder = device->create_command_encoder("upload Blit vertex buffer");
    encoder->write_buffer(vertex_buffer, 0, sizeof(vertices), vertices);
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
            Descriptor::sampled(0, ShaderStage::Fragment, "uTexture"),
        });

        pipeline = device->create_render_pipeline(
            device->create_shader_module(vert_source, ShaderStage::Vertex, "blit vert"),
            device->create_shader_module(frag_source, ShaderStage::Fragment, "blit frag"),
            attribute_descriptions,
            blend_state,
            descriptor_set,
            render_pass,
            "blit pipeline");
    }
}

void Blit::set_texture(const std::shared_ptr<Texture> &new_texture) {
    texture = new_texture;

    descriptor_set->add_or_update({
        Descriptor::sampled(0, ShaderStage::Fragment, "uTexture", texture, sampler),
    });
}

void Blit::draw(const std::shared_ptr<CommandEncoder> &encoder) {
    encoder->bind_render_pipeline(pipeline);

    encoder->bind_vertex_buffers({vertex_buffer});

    encoder->bind_descriptor_set(descriptor_set);

    encoder->draw(0, 6);
}
