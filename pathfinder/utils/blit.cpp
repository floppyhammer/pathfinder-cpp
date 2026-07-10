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
                DescriptorLayout{0, ShaderStage::Fragment, DescriptorType::Sampler},
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
        Descriptor::sampled(0, texture_, sampler_),
    });
}

void Blit::draw(const std::shared_ptr<CommandEncoder> &encoder) {
    encoder->bind_render_pipeline(pipeline_);

    encoder->bind_descriptor_set(descriptor_set_);

    encoder->draw(0, 3);
}
