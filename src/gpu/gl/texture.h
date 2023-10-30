#ifndef PATHFINDER_GPU_TEXTURE_GL_H
#define PATHFINDER_GPU_TEXTURE_GL_H

#include "../../common/global_macros.h"
#include "../../common/logger.h"
#include "../../common/math/rect.h"
#include "../texture.h"
#include "data.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

/// Use Texture via smart pointers as its de-constructor will release its GL resources.
class TextureGl : public Texture {
    friend class DeviceGl;

public:
    explicit TextureGl(const TextureDescriptor& _desc);

    ~TextureGl() override;

    uint32_t get_texture_id() const;

    void set_label(const std::string& _label) override;

private:
    uint32_t texture_id = 0;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_TEXTURE_GL_H
