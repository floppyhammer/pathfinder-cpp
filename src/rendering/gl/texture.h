//
// Created by floppyhammer on 6/7/2021.
//

#ifndef PATHFINDER_TEXTURE_H
#define PATHFINDER_TEXTURE_H

#include "data.h"
#include "../../common/math/rect.h"
#include "../../common/global_macros.h"
#include "../../common/logger.h"

#include "stb_image.h"

namespace Pathfinder {
    /// Use Texture via smart pointers as its de-constructor will release its GL resources.
    class Texture {
    public:
        Texture(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type);

        ~Texture();

        uint32_t get_texture_id() const;

        uint32_t get_width() const;

        uint32_t get_height() const;

        Vec2<uint32_t> get_size() const;

        TextureFormat get_format() const;

        DataType get_pixel_type() const;

    private:
        uint32_t texture_id = 0;

        uint32_t width = 0;
        uint32_t height = 0;

        /// Pixel data type (GPU).
        TextureFormat format;

        /// Pixel data type (CPU).
        DataType type = DataType::UNSIGNED_BYTE;
    };
}

#endif //PATHFINDER_TEXTURE_H
