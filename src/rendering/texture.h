//
// Created by floppyhammer on 6/7/2021.
//

#ifndef PATHFINDER_TEXTURE_H
#define PATHFINDER_TEXTURE_H

#include "device.h"
#include "base.h"
#include "../common/math/rect.h"
#include "../common/global_macros.h"
#include "../common/logger.h"

#include <stb_image.h>

namespace Pathfinder {
    /// Use Texture via smart pointers as its de-constructor will release its GL resources.
    class Texture {
    public:
        Texture(int p_width, int p_height, TextureFormat p_format, DataType p_type, const void *p_data = nullptr);

        ~Texture();

        /// From file.
        static std::shared_ptr<Texture> from_file(const char *file_path,
                                                  TextureFormat p_format,
                                                  DataType p_type,
                                                  bool flip_y);

        /// From memory.
        static std::shared_ptr<Texture> from_memory(const std::vector<unsigned char> &bytes,
                                                    TextureFormat p_format,
                                                    DataType p_type,
                                                    bool flip_y);

        unsigned int get_texture_id() const;

        unsigned int get_width() const;

        unsigned int get_height() const;

        TextureFormat get_format() const;

        DataType get_pixel_type() const;

        void update_region(const Rect<int> &p_rect, const void *p_data) const;

    private:
        unsigned int texture_id = 0;

        int width = 0;
        int height = 0;

        /// Pixel data type (GPU).
        TextureFormat format;

        /// Pixel data type (CPU).
        DataType type = DataType::UNSIGNED_BYTE;
    };
}

#endif //PATHFINDER_TEXTURE_H
