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

    void set_label(const std::string& label) override;

private:
    explicit TextureGl(const TextureDescriptor& desc);

    uint32_t gl_id_ = 0;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_TEXTURE_GL_H
