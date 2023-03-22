#include "gradient.h"

#include "../../common/math/basic.h"

namespace Pathfinder {

const float COLOR_STOP_OFFSET_TOL = 1e-3;

void Gradient::add(const ColorStop &_stop) {
    auto iter = stops.begin();

    // Find a place to insert the new stop.
    while (iter != stops.end() && (iter->offset < _stop.offset)) {
        ++iter;
    }

    stops.insert(iter, _stop);
}

void Gradient::add_color_stop(const ColorU &color, float offset) {
    add(ColorStop{offset, color});
}

ColorU Gradient::sample(float t) const {
    if (stops.empty()) {
        return ColorU::transparent_black();
    }

    t = clamp(t, 0.0f, 1.0f);

    size_t last_index = stops.size() - 1;

    auto iter = stops.begin();

    while (iter != stops.end() && (iter->offset < t || iter->offset == 0)) {
        ++iter;
    }

    size_t upper_index = std::min(size_t(iter - stops.begin()), last_index);

    size_t lower_index = upper_index > 0 ? upper_index - 1 : upper_index;

    auto &lower_stop = stops[lower_index];
    auto &upper_stop = stops[upper_index];

    float denom = upper_stop.offset - lower_stop.offset;

    // Handle stops with the same offset.
    // No need to consider float precision here.
    if (denom == 0) {
        return lower_stop.color;
    }

    float ratio = std::min((t - lower_stop.offset) / denom, 1.0f);

    return ColorU(lower_stop.color.to_f32().lerp(upper_stop.color.to_f32(), ratio));
}

bool Gradient::is_opaque() {
    bool opaque = false;

    for (auto &stop : stops) {
        opaque |= stop.color.is_opaque();
    }

    return opaque;
}

TextureLocation GradientTileBuilder::allocate(const Gradient &gradient,
                                              TextureAllocator &allocator,
                                              std::vector<TextureLocation> transient_paint_locations) {
    // Allocate new texture page.
    if (tiles.empty() || tiles.back().next_index == GRADIENT_TILE_LENGTH) {
        auto size = Vec2I(GRADIENT_TILE_LENGTH);
        uint32_t area = size.area();

        auto page_location = allocator.allocate(size, AllocationMode::OwnPage);
        transient_paint_locations.push_back(page_location);

        // New tile.
        tiles.push_back(GradientTile{
            std::vector<ColorU>(area, ColorU::black()),
            page_location.page,
            0,
        });
    }

    auto &tile = tiles.back();

    // Texture location that we should write to.
    auto location = TextureLocation{
        tile.page,
        RectI(0, tile.next_index, GRADIENT_TILE_LENGTH, tile.next_index + 1),
    };

    // Update which row we should write next.
    tile.next_index += 1;

    // FIXME(pcwalton): Paint transparent if gradient line has zero size, per spec.
    // TODO(pcwalton): Optimize this:
    // 1. Calculate ∇t up front and use differencing in the inner loop.
    // 2. Go four pixels at a time with SIMD.
    auto first_address = location.rect.origin().y * GRADIENT_TILE_LENGTH;

    // Update pixel column by column.
    for (int x = 0; x < GRADIENT_TILE_LENGTH; x++) {
        auto t = ((float)x + 0.5f) / GRADIENT_TILE_LENGTH;
        tile.texels[first_address + x] = gradient.sample(t);
    }

    return location;
}

} // namespace Pathfinder
