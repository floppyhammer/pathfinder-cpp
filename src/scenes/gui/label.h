//
// Created by floppyhammer on 7/19/2021.
//

#ifndef PATHFINDER_LABEL_H
#define PATHFINDER_LABEL_H

#include "control.h"
#include "../../modules/text/font.h"
#include "../../modules/d3d9_d3d11/canvas.h"
#include "../../modules/d3d9_d3d11/scene.h"
#include "../../modules/d3d9/scene_builder.h"
#include "../../modules/d3d9/renderer.h"

#include <cstdint>

namespace Pathfinder {
    class Label : public Control {
    public:
        Label(unsigned int viewport_width, unsigned int viewport_height, const std::vector<unsigned char> &area_lut_input);

        /**
         * Set text context.
         * @note See https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html for glyph conventions.
         * @param p_text Text string.
         */
        void set_text(const std::string &p_text);

        void set_font(std::shared_ptr<Font> p_font);

        void set_style(float p_size, ColorU p_color, float p_stroke_width, ColorU p_stroke_color);

        void draw();

        bool debug = false;

        std::shared_ptr<Canvas> canvas;

    private:
        std::string text;

        std::shared_ptr<Font> font;

        float line_height = 64;

        bool is_dirty = false;

        struct Glyph {
            int start = -1; // Start offset in the source string.
            int end = -1; // End offset in the source string.

            uint8_t count = 0; // Number of glyphs in the grapheme, set in the first glyph only.
            uint8_t repeat = 1; // Draw multiple times in the row.
            uint16_t flags = 0; // Grapheme flags (valid, rtl, virtual), set in the first glyph only.

            float x_off = 0.f; // Offset from the origin of the glyph on baseline.
            float y_off = 0.f;
            float advance = 0.f; // Advance to the next glyph along baseline(x for horizontal layout, y for vertical).

            int font_size = 0; // Font size;
            char32_t text{};
            int32_t index = 0; // Glyph index (font specific) or UTF-32 codepoint (for the invalid glyphs).

            Rect<float> layout_box;
            Rect<float> bbox;

            Shape shape; // Glyph shape.
        };

        std::vector<Glyph> glyphs;

        ColorU color;

        float stroke_width = 0;
        ColorU stroke_color;

        void update();
    };
}

#endif //PATHFINDER_LABEL_H
