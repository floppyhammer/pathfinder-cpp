#include "sampler.h"

#include "base.h"

namespace Pathfinder {

SamplerGl::SamplerGl(SamplerDescriptor descriptor) : Sampler(descriptor) {
    glGenSamplers(1, &id_);

    glSamplerParameteri(id_, GL_TEXTURE_WRAP_S, to_gl_sampler_address_mode(descriptor.address_mode_u));
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_T, to_gl_sampler_address_mode(descriptor.address_mode_v));
    glSamplerParameteri(id_, GL_TEXTURE_MIN_FILTER, to_gl_sampler_filter(descriptor.min_filter));
    glSamplerParameteri(id_, GL_TEXTURE_MAG_FILTER, to_gl_sampler_filter(descriptor.mag_filter));
}

SamplerGl::~SamplerGl() {
    glDeleteSamplers(1, &id_);
}

} // namespace Pathfinder
