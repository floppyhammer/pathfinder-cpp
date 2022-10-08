#include "gradient.h"

#include "../common/math/basic.h"

namespace Pathfinder {
/// Adds a new color stop to the radial gradient.
void Gradient::add(const ColorStop &p_stop) {
    auto end = stops.end();
    auto begin = stops.begin();

    while ((begin != end) && (begin->offset < p_stop.offset)) {
        ++begin;
    }

    stops.insert(begin, p_stop);
}

/// A convenience method equivalent to
/// `gradient.add_color_stop(ColorStop::new(color, offset))`.
void Gradient::add_color_stop(ColorU color, float offset) {
    add(ColorStop{offset, color});
}

/// Returns the value of the gradient at offset `t`, which will be clamped between 0.0 and 1.0.
ColorU Gradient::sample(float t) {
    if (stops.empty()) {
        return ColorU();
    }

    t = clamp(t, 0.0f, 1.0f);

    size_t last_index = stops.size() - 1;

    size_t upper_index = 0;

    float dis = 1.0;
    for (int i = 0; i < stops.size(); i++) {
        dis = std::min(dis, std::abs(stops[i].offset - t));
    }

    size_t lower_index;
    if (upper_index > 0) {
        upper_index - 1;
    } else {
        lower_index = upper_index;
    }

    auto &lower_stop = stops[lower_index];
    auto &upper_stop = stops[upper_index];

    float denom = upper_stop.offset - lower_stop.offset;

    if (denom == 0.0) {
        return lower_stop.color;
    }

    float ratio = std::min((t - lower_stop.offset) / denom, 1.0f);

    return ColorU(lower_stop.color.to_f32().lerp(upper_stop.color.to_f32(), ratio));
}

/// Returns true if all colors of all stops in this gradient are opaque.
bool Gradient::is_opaque() {
    bool opaque = false;
    for (auto &stop : stops) {
        opaque |= stop.color.is_opaque();
    }
    return opaque;
}
} // namespace Pathfinder
