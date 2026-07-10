#include "blit.h"

/* clang-format off */
// SPV
#include "../shaders/generated/blit_frag_spv.h"
#include "../shaders/generated/blit_vert_spv.h"
// GLSL
#include "../shaders/generated/blit_frag.h"
#include "../shaders/generated/blit_vert.h"
/* clang-format on */

using namespace Pathfinder;

Blit::Blit(const std::shared_ptr<Device> &device, const std::shared_ptr<Queue> &queue, TextureFormat target_format) {
    device_ = device;
    queue_ = queue;

    sampler_ = device->create_sampler(SamplerDescriptor{});

    // Pipeline.
    {
        std::vector<char> vert_source, frag_source;

        if (device_->get_backend_type() == BackendType::Vulkan) {
            vert_source = std::vector<char>(std::begin(blit_vert_spv), std::end(blit_vert_spv));
            frag_source = std::vector<char>(std::begin(blit_frag_spv), std::end(blit_frag_spv));
        } else {
            vert_source = std::vector<char>(std::begin(blit_vert), std::end(blit_vert));
            frag_source = std::vector<char>(std::begin(blit_frag), std::end(blit_frag));
        }

        const auto blend_state = BlendState::from_over();

        {
            std::vector<DescriptorLayout> layouts = {
                DescriptorLayout{0, ShaderStage::Fragment, DescriptorType::Sampler, "uTexture"},
            };

            descriptor_set_layout_ = device->create_descriptor_set_layout(layouts);
        }

        descriptor_set_ = device->create_descriptor_set(descriptor_set_layout_);

        pipeline_ = device->create_render_pipeline(
            device->create_shader_module(vert_source, ShaderStage::Vertex, "blit vert"),
            device->create_shader_module(frag_source, ShaderStage::Fragment, "blit frag"),
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
