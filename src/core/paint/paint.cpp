#include "paint.h"

namespace Pathfinder {

RectF rect_to_uv(const RectI &rect, const Vec2F &texture_scale) {
    return rect.to_f32() * texture_scale;
}

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

ColorU Paint::get_base_color() const {
    return base_color;
}

void Paint::set_base_color(const ColorU &color) {
    base_color = color;
}

std::shared_ptr<PaintOverlay> Paint::get_overlay() const {
    return overlay;
}

PaintFilter PaintMetadata::filter() const {
    if (!color_texture_metadata) {
        return PaintFilter{};
    }

    PaintFilter filter = color_texture_metadata->filter;

    switch (color_texture_metadata->filter.type) {
        case PaintFilter::Type::RadialGradient: {
            auto uv_rect = rect_to_uv(color_texture_metadata->location.rect, color_texture_metadata->page_scale);

            // Contract rect.
            auto amount = Vec2F(0.0, color_texture_metadata->page_scale.y * 0.5f);
            uv_rect = RectF(uv_rect.origin() + amount, uv_rect.lower_right() - amount);

            filter.gradient_filter.uv_origin = uv_rect.origin();
        } break;
        default:
            break;
    }

    return filter;
}

std::shared_ptr<Texture> PaintMetadata::tile_batch_texture() const {
    if (color_texture_metadata) {
        return color_texture_metadata->color_texture;
    }

    return nullptr;
}

} // namespace Pathfinder
