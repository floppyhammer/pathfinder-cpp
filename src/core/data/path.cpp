#include "path.h"

#include "../../common/math/basic.h"

namespace Pathfinder {

void Outline::transform(const Transform2 &transform) {
    if (transform.is_identity()) {
        return;
    }

    Rect<float> new_bounds;

    for (auto &contour : contours) {
        contour.transform(transform);

        // Update bounds.
        new_bounds = new_bounds.union_rect(contour.bounds);
    }

    bounds = new_bounds;
}

void Outline::push_contour(const Contour &p_contour) {
    if (p_contour.is_empty()) {
        return;
    }

    // Push contour.
    contours.push_back(p_contour);

    // Update bounds.
    if (contours.empty()) {
        bounds = p_contour.bounds;
    } else {
        bounds = bounds.union_rect(p_contour.bounds);
    }
}

} // namespace Pathfinder
