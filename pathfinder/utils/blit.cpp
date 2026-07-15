#include "blit.h"

/* clang-format off */
#include "../gpu/shader.h"
#include "../shaders/generated/blit_frag_shdbin.h"
#include "../shaders/generated/blit_vert_shdbin.h"
/* clang-format on */

using namespace Pathfinder;

Blit::Blit(const std::shared_ptr<Device> &device, const std::shared_ptr<Queue> &queue, TextureFormat target_format) {
    device_ = device;
    queue_ = queue;

    sampler_ = device->create_sampler(SamplerDescriptor{});

    // Pipeline.
    {
        auto vert_shader = Shader::create_from_shdbin(blit_vert_shdbin, sizeof(blit_vert_shdbin));
        auto frag_shader = Shader::create_from_shdbin(blit_frag_shdbin, sizeof(blit_frag_shdbin));

        const auto blend_state = BlendState::from_over();

        {
            std::vector<DescriptorLayout> layouts = {
                DescriptorLayout{0, ShaderStage::Vertex, DescriptorType::UniformBuffer},
                DescriptorLayout{1, ShaderStage::Fragment, DescriptorType::Sampler},
            };

            descriptor_set_layout_ = device->create_descriptor_set_layout(layouts);
        }

        descriptor_set_ = device->create_descriptor_set(descriptor_set_layout_);

        pipeline_ = device->create_render_pipeline(device->create_shader_module(vert_shader, "blit vert"),
                                                   device->create_shader_module(frag_shader, "blit frag"),
                                                   {},
                                                   blend_state,
                                                   descriptor_set_layout_,
                                                   target_format,
                                                   "blit pipeline");
    }
}

void Blit::set_texture(const std::shared_ptr<Texture> &new_texture) {
    texture_ = new_texture;

    descriptor_set_->add_or_update({
        Descriptor::sampled(1, texture_, sampler_),
    });
}

void Blit::draw(const std::shared_ptr<CommandEncoder> &encoder) {
    if (!uniform_buffer_) {
        auto descriptor =
            BufferDescriptor{BufferType::Uniform, 4 * sizeof(float), MemoryProperty::HostVisibleAndCoherent};

        uniform_buffer_ = device_->create_buffer(descriptor, "blit uniform");

        float flip_y = 1.0f;
        if (device_->get_backend_type() == BackendType::Opengl) {
            flip_y = -1.0f;
        }

        uniform_buffer_->upload_via_mapping(sizeof(float), 0, &flip_y);

        descriptor_set_->add_or_update({Descriptor::uniform(0, uniform_buffer_)});
    }

    encoder->bind_render_pipeline(pipeline_);

    encoder->bind_descriptor_set(descriptor_set_);

    encoder->draw(0, 3);
}
