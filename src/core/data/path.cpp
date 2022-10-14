#include "path.h"

#include "../../common/math/basic.h"

namespace Pathfinder {

void Outline::scale(const Vec2<float> &scale) {
    // Outline origin.
    // Vec2<float> origin = bounds.origin();
    Vec2<float> origin{};

    for (auto &contour : contours) {
        for (auto &point : contour.points) {
            // Move to local coordinates first.
            point -= origin;

            // Do scaling.
            point *= scale;

            // Move it back to global coordinates.
            point += origin;
        }
    }

    update_bounds();
}

void Outline::translate(const Vec2<float> &translation) {
    for (auto &contour : contours) {
        for (auto &point : contour.points) {
            point += translation;
        }
    }

    update_bounds();
}

void Outline::rotate(float rotation) {
    // Outline origin.
    // Vec2<float> origin = bounds.origin();
    Vec2<float> origin{};

    float rotation_in_radian = deg2rad(rotation);

    for (auto &contour : contours) {
        for (auto &point : contour.points) {
            // Move to local coordinates first.
            point -= origin;

            // Do rotation.
            point *= Vec2<float>(cos(rotation_in_radian), sin(rotation_in_radian));

            // Move it back to global coordinates.
            point += origin;
        }
    }

    update_bounds();
}

void Outline::transform(const Transform2 &transform) {
    for (auto &contour : contours) {
        for (auto &point : contour.points) {
            point = transform * point;
        }
    }

    update_bounds();
}

void Outline::update_bounds() {
    bounds = Rect<float>();

    // Update child contours' bounds and its own bounds.
    for (auto &contour : contours) {
        contour.bounds = Rect<float>();

        for (auto &point : contour.points) {
            union_rect(contour.bounds, point);
        }

        // Own bounds.
        bounds = bounds.union_rect(contour.bounds);
    }
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
