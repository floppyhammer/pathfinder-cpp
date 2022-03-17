//
// Created by chy on 7/19/2021.
//

#include "label.h"

#include <string>

namespace Pathfinder {
    Label::Label(unsigned int viewport_width, unsigned int viewport_height, const std::vector<unsigned char> &area_lut_input) {
        canvas = std::make_shared<Canvas>((float) viewport_width, (float) viewport_height, area_lut_input);
    }

    void Label::set_text(const std::string &p_text) {
        // Only update glyphs when text has changed.
        if (text == p_text || font == nullptr) return;

        text = p_text;

        // Get font info. Get font scaling.
        int ascent, descent, line_gap;
        float scale = font->get_metrics(line_height, ascent, descent, line_gap);

        // Convert text string to utf32 string.
        std::u32string utf32_str(text.begin(), text.end());

        // Offset.
        float x = 0, y = 0;

        glyphs.clear();
        glyphs.reserve(utf32_str.size());

        for (char32_t u_codepoint: utf32_str) {
            Glyph g;

            // Set UTF-32 codepoint.
            g.text = u_codepoint;

            // Set glyph index.
            g.index = stbtt_FindGlyphIndex(&font->info, (int) u_codepoint);

            g.x_off = x;
            g.y_off = y;

            if (u_codepoint == '\n') {
                x = 0;
                y += line_height;
                glyphs.push_back(g);
                continue;
            }

            // The horizontal distance to increment (for left-to-right writing) or decrement (for right-to-left writing)
            // the pen position after a glyph has been rendered when processing text.
            // It is always positive for horizontal layouts, and zero for vertical ones.
            int advance_width;

            // The horizontal distance from the current pen position to the glyph's left bbox edge.
            // It is positive for horizontal layouts, and in most cases negative for vertical ones.
            int left_side_bearing;

            stbtt_GetGlyphHMetrics(&font->info, g.index, &advance_width, &left_side_bearing);

            g.advance = (float) advance_width * scale;

            // Get bounding box for character (maybe offset to account for chars that dip above or below the line).
            Rect<int> bounding_box;
            stbtt_GetGlyphBitmapBox(&font->info, g.index, scale, scale,
                                    &bounding_box.left,
                                    &bounding_box.top,
                                    &bounding_box.right,
                                    &bounding_box.bottom);

            // Compute baseline height (different characters have different heights).
            float local_y = ascent + bounding_box.top;

            // Offset
            float byte_offset = x + roundf(left_side_bearing * scale) + (y * rect_size.y);

            // Set glyph shape.
            // --------------------------------
            g.shape = font->get_glyph_shape(g.index);

            g.shape.scale(Vec2<float>(scale, -scale));
            g.shape.translate(Vec2<float>(x, line_height + descent + y));
            // --------------------------------

            // Layout box.
            g.layout_box = Rect<float>(x, line_height + descent - ascent + y, x + advance_width * scale,
                                       line_height + y);

            // Bbox.
            g.bbox = Rect<float>(x + bounding_box.left, line_height + descent + bounding_box.bottom + y,
                                 x + bounding_box.right, line_height + descent + bounding_box.top + y);

            // Advance x.
            x += roundf(advance_width * scale);

            glyphs.push_back(g);
        }

        // Mark the label as dirty, so it needs to be redrawn.
        is_dirty = true;
    }

    void Label::set_font(std::shared_ptr<Font> p_font) {
        font = std::move(p_font);
    }

    void Label::update() {
        canvas->clear();

        // Add stroke.
        for (Glyph &g: glyphs) {
            // Add stroke if needed.
            canvas->set_stroke_paint(Paint::from_color(ColorU(stroke_color)));
            canvas->set_line_width(stroke_width);
            canvas->stroke_shape(g.shape);
        }

        for (Glyph &g: glyphs) {
            if (debug) {
                canvas->set_line_width(1);

                // Add layout box.
                // --------------------------------
                Shape layout_shape;
                layout_shape.add_rect(g.layout_box);

                canvas->set_stroke_paint(Paint::from_color(ColorU::green()));
                canvas->stroke_shape(layout_shape);
                // --------------------------------

                // Add bbox.
                // --------------------------------
                Shape bbox_shape;
                bbox_shape.add_rect(g.bbox);

                canvas->set_stroke_paint(Paint::from_color(ColorU::red()));
                canvas->stroke_shape(bbox_shape);
                // --------------------------------
            }

            canvas->set_fill_paint(Paint::from_color(ColorU(color)));
            canvas->fill_shape(g.shape, FillRule::Winding);
        }

        is_dirty = false;

        canvas->update();
    }

    void Label::set_style(float p_size, ColorU p_color, float p_stroke_width, ColorU p_stroke_color) {
        line_height = p_size;
        color = p_color;
        stroke_width = p_stroke_width;
        stroke_color = p_stroke_color;

        is_dirty = true;
    }

    void Label::draw() {
        if (is_dirty) update();

        canvas->draw();
    }
}
