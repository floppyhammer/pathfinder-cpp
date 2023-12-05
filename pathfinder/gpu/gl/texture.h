#ifndef PATHFINDER_GPU_TEXTURE_GL_H
#define PATHFINDER_GPU_TEXTURE_GL_H

#include "../texture.h"
#include "base.h"

namespace Pathfinder {

/// Use Texture via smart pointers as its de-constructor will release its GL resources.
class TextureGl : public Texture {
    friend class DeviceGl;

public:
    explicit TextureGl(const TextureDescriptor& desc);

    ~TextureGl() override;

    uint32_t get_texture_id() const;

    void set_label(const std::string& label) override;

private:
    uint32_t texture_id_ = 0;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_TEXTURE_GL_H
