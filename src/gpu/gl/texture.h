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
public:
    TextureGl(Vec2I _size, TextureFormat _format, std::string _label);

    ~TextureGl();

    uint32_t get_texture_id() const;

private:
    uint32_t texture_id = 0;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_TEXTURE_GL_H
