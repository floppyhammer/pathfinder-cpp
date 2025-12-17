#ifndef PATHFINDER_GPU_TEXTURE_GL_H
#define PATHFINDER_GPU_TEXTURE_GL_H

#include "../texture.h"
#include "base.h"

namespace Pathfinder {

/// Use Texture via smart pointers as its de-constructor will release its GL resources.
class TextureGl : public Texture {
    friend class DeviceGl;

public:
    ~TextureGl() override;

    uint32_t get_texture_id() const;

    uint32_t get_pbo_id() const;

    void set_label(const std::string& label) override;

    void prepare_pbo();

private:
    explicit TextureGl(const TextureDescriptor& desc);

    TextureGl(uint32_t external_gl_id, const TextureDescriptor& desc);

    bool wrapped = false;

    uint32_t gl_id_ = 0;

    uint32_t pbo_id_ = 0;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_TEXTURE_GL_H
