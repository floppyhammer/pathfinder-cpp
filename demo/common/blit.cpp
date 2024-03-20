#include "blit.h"

#ifdef PATHFINDER_USE_VULKAN
    #include "../../pathfinder/shaders/generated/blit_frag_spv.h"
    #include "../../pathfinder/shaders/generated/blit_vert_spv.h"
#else
    #include "../../pathfinder/shaders/generated/blit_frag.h"
    #include "../../pathfinder/shaders/generated/blit_vert.h"
#endif

using namespace Pathfinder;

Blit::Blit(const std::shared_ptr<Device> &device, const std::shared_ptr<Queue> &queue, TextureFormat target_format) {
    device_ = device;
    queue_ = queue;

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

    vertex_buffer_ = device->create_buffer({BufferType::Vertex, sizeof(vertices), MemoryProperty::DeviceLocal},
                                           "blit vertex buffer");

    sampler_ = device->create_sampler(SamplerDescriptor{});

    auto encoder = device->create_command_encoder("upload Blit vertex buffer");
    encoder->write_buffer(vertex_buffer_, 0, sizeof(vertices), vertices);
    queue_->submit_and_wait(encoder);

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

        descriptor_set_ = device->create_descriptor_set();
        descriptor_set_->add_or_update({
            Descriptor::sampled(0, ShaderStage::Fragment, "uTexture"),
        });

        pipeline_ = device->create_render_pipeline(
            device->create_shader_module(vert_source, ShaderStage::Vertex, "blit vert"),
            device->create_shader_module(frag_source, ShaderStage::Fragment, "blit frag"),
            attribute_descriptions,
            blend_state,
            descriptor_set_,
            target_format,
            "blit pipeline");
    }
}

void Blit::set_texture(const std::shared_ptr<Texture> &new_texture) {
    texture_ = new_texture;

    descriptor_set_->add_or_update({
        Descriptor::sampled(0, ShaderStage::Fragment, "uTexture", texture_, sampler_),
    });
}

void Blit::draw(const std::shared_ptr<CommandEncoder> &encoder) {
    encoder->bind_render_pipeline(pipeline_);

    encoder->bind_vertex_buffers({vertex_buffer_});

    encoder->bind_descriptor_set(descriptor_set_);

    encoder->draw(0, 6);
}
