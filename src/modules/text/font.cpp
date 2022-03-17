//
// Created by chy on 9/1/2021.
//

#include "font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <vector>

namespace Pathfinder {
    Font::Font(std::vector<char> &bytes) {
        auto byte_size = bytes.size() * sizeof(unsigned char);
        buffer = static_cast<unsigned char *>(malloc(byte_size));
        memcpy(buffer, bytes.data(), byte_size);

        // Prepare font info.
        if (!stbtt_InitFont(&info, buffer, 0)) {
            Logger::error("Failed to prepare font info!", "Font");
        }
    }

    Font::~Font() {
        free(buffer);
    }

    float Font::get_metrics(float line_height, int &ascent, int &descent, int &line_gap) const {
        // Calculate font scaling.
        float scale = stbtt_ScaleForPixelHeight(&info, line_height);

        stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);

        // Take scale into account.
        ascent = int(roundf(float(ascent) * scale));
        descent = int(roundf(float(descent) * scale));

        return scale;
    }

    Shape Font::get_glyph_shape(int glyph_index) const {
        Shape shape;

        stbtt_vertex *vertices = nullptr;

        int num_vertices = stbtt_GetGlyphShape(&info, glyph_index, &vertices);

        if (vertices == nullptr) {
            Logger::warn("Failed to get glyph shape!", "Font");
            return shape;
        }

        for (int i = 0; i < num_vertices; i++) {
            auto &v = vertices[i];

            switch (v.type) {
                case STBTT_vmove:
                    // Close last path (if there's any).
                    shape.close();

                    shape.move_to(v.x, v.y);
                    break;
                case STBTT_vline:
                    shape.line_to(v.x, v.y);
                    break;
                case STBTT_vcurve:
                    shape.curve_to(v.cx, v.cy, v.x, v.y);
                    break;
                case STBTT_vcubic:
                    shape.cubic_to(v.cx, v.cy, v.cx1, v.cy1, v.x, v.y);
                    break;
            }
        }

        // Close last path.
        shape.close();

        shape.fill_rule = FillRule::EvenOdd;

        stbtt_FreeShape(&info, vertices);

        return shape;
    }
}
