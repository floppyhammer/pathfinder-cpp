#include "paint.h"

#include <stdexcept>

namespace Pathfinder {

Rect<float> rect_to_uv(Rect<uint32_t> rect, Vec2<float> texture_scale) {
    return rect.to_f32() * texture_scale;
}

// Paint member functions.
// ---------------------------------------------------
/// Returns true if this paint is obviously opaque, via a quick check.
bool Paint::is_opaque() const {
    if (!base_color.is_opaque()) {
        return false;
    }

    if (overlay) {
        auto &content = overlay->contents;
        if (content.type == PaintContents::Type::Gradient) {
            return content.gradient.is_opaque();
        } else {
            return content.pattern.is_opaque();
        }
    }

    return true;
}

/// Returns the *base color* of this paint.
ColorU Paint::get_base_color() const {
    return base_color;
}

/// Changes the *base color* of this paint.
void Paint::set_base_color(ColorU p_color) {
    base_color = p_color;
}

std::shared_ptr<PaintOverlay> Paint::get_overlay() const {
    return overlay;
}
// ---------------------------------------------------

PaintFilter PaintMetadata::filter() const {
    if (!color_texture_metadata) {
        return PaintFilter{};
    }

    PaintFilter filter = color_texture_metadata->filter;

    switch (color_texture_metadata->filter.type) {
        case PaintFilter::Type::RadialGradient: {
            auto uv_rect = rect_to_uv(color_texture_metadata->location.rect, color_texture_metadata->page_scale);
            filter.gradient_filter.uv_origin = uv_rect.origin();
        } break;
        default:
            break;
    }

    return filter;
}
} // namespace Pathfinder
