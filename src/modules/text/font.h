//
// Created by chy on 9/1/2021.
//

#ifndef PATHFINDER_FONT_H
#define PATHFINDER_FONT_H

#include <stb_truetype.h>

#include "../d3d9_d3d11/data/shape.h"
#include "../../common/logger.h"
#include "../../common/io.h"

#include <cstdio>
#include <cstdlib>

namespace Pathfinder {
    class Font {
    public:
        explicit Font(std::vector<char> &bytes);
        ~Font();

        static std::shared_ptr<Font> from_file(const char *file_path) {
            std::shared_ptr<Font> font;

            auto font_buffer = load_file_as_bytes(file_path);

            font = std::make_shared<Font>(font_buffer);

            return font;
        }

        stbtt_fontinfo info{};

        float get_metrics(float line_height, int &ascent, int &descent, int &line_gap) const;

        Shape get_glyph_shape(int glyph_index) const;

    private:
        /// Stores font data, should not be freed until font is deleted.
        unsigned char *buffer;
    };
}

#endif //PATHFINDER_FONT_H
