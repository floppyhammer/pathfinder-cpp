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
        return std::visit([](auto &&content) { return content.is_opaque(); }, overlay->contents);
    }

    return true;
}

bool Paint::is_visible() const {
    if (!base_color.is_visible()) {
        return false;
    }

    if (!overlay) {
        return true;
    }

    return std::visit([](auto &&content) { return content.is_visible(); }, overlay->contents);
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

    if (std::holds_alternative<RadialGradientPaintFilter>(filter)) {
        auto &rg = std::get<RadialGradientPaintFilter>(filter);

        auto uv_rect = rect_to_uv(color_texture_metadata->location.rect, color_texture_metadata->page_scale);

        // Contract rect.
        auto amount = Vec2F(0.0, color_texture_metadata->page_scale.y * 0.5f);
        uv_rect = RectF(uv_rect.origin() + amount, uv_rect.lower_right() - amount);

        rg.uv_origin = uv_rect.origin();
    }

    return filter;
}

std::shared_ptr<TileBatchTextureInfo> PaintMetadata::tile_batch_texture_info() const {
    if (color_texture_metadata) {
        auto info = std::make_shared<TileBatchTextureInfo>();
        info->page_id = color_texture_metadata->location.page;
        info->sampling_flags = color_texture_metadata->sampling_flags;
        info->composite_op = color_texture_metadata->composite_op;
        info->raw_texture = color_texture_metadata->raw_texture;
        return info;
    }

    return nullptr;
}

} // namespace Pathfinder
