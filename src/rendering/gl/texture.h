#ifndef PATHFINDER_TEXTURE_GL_H
#define PATHFINDER_TEXTURE_GL_H

#include "data.h"
#include "../texture.h"
#include "../../common/math/rect.h"
#include "../../common/global_macros.h"
#include "../../common/logger.h"

#include "stb_image.h"

namespace Pathfinder {
    /// Use Texture via smart pointers as its de-constructor will release its GL resources.
    class TextureGl : public Texture {
    public:
        TextureGl(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type);

        ~TextureGl();

        uint32_t get_texture_id() const;

    private:
        uint32_t texture_id = 0;
    };
}

#endif //PATHFINDER_TEXTURE_GL_H
