#include "path.h"

#include "../../common/math/basic.h"

namespace Pathfinder {

void Outline::transform(const Transform2 &transform) {
    if (transform.is_identity()) {
        return;
    }

    RectF new_bounds;

    for (auto &contour : contours) {
        contour.transform(transform);

        // Update bounds.
        new_bounds = new_bounds.union_rect(contour.bounds);
    }

    bounds = new_bounds;
}

void Outline::push_contour(const Contour &_contour) {
    if (_contour.is_empty()) {
        return;
    }

    // Push contour.
    contours.push_back(_contour);

    // Update bounds.
    if (contours.empty()) {
        bounds = _contour.bounds;
    } else {
        bounds = bounds.union_rect(_contour.bounds);
    }
}

} // namespace Pathfinder
