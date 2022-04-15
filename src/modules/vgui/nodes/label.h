//
// Created by floppyhammer on 7/19/2021.
//

#ifndef PATHFINDER_LABEL_H
#define PATHFINDER_LABEL_H

#include "control.h"
#include "../resources/style_box.h"
#include "../resources/font.h"

#include <cstdint>

namespace Pathfinder {
    enum class Alignment {
        Begin,
        Center,
        End,
    };

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

    class Label : public Control {
    public:
        /**
         * Set text context.
         * @note See https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html for glyph conventions.
         * @param p_text Text string.
         */
        void set_text(const std::string &p_text);

        void set_font(std::shared_ptr<Font> p_font);

        void set_style(float p_size, ColorU p_color, float p_stroke_width, ColorU p_stroke_color);

        void update() override;

        void draw() override;

        bool debug = false;

        void set_horizontal_alignment(Alignment alignment);

        void set_vertical_alignment(Alignment alignment);

    private:
        void measure();

        void adjust_layout();

    private:
        std::string text;

        std::shared_ptr<Font> font;

        float line_height = 64;

        bool is_dirty = false;

        std::vector<Glyph> glyphs;

        Rect<float> layout_box;

        // Fill
        ColorU color;

        // Stroke
        float stroke_width = 0;
        ColorU stroke_color;

        // Layout
        Alignment horizontal_alignment = Alignment::Begin;
        Alignment vertical_alignment = Alignment::Begin;
    };
}

#endif //PATHFINDER_LABEL_H
