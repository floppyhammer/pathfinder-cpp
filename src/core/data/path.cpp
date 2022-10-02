#include "path.h"

#include "../../common/math/basic.h"

namespace Pathfinder {
const float RATIO = 0.552284749831; // 4.0f * (sqrt(2.0f) - 1.0f) / 3.0f

void Outline::move_to(float x, float y) {
    // Add a new empty contour.
    contours.emplace_back();

    auto &current_contour = contours.back();

    // First on-curve point.
    contours.back().points.emplace_back(x, y);
    contours.back().flags.emplace_back();

    // Update contour bounds.
    union_rect(current_contour.bounds, Vec2<float>(x, y), true);

    // Update path bounds.
    bounds = bounds.union_rect(current_contour.bounds);
}

void Outline::line_to(float x, float y) {
    if (contours.empty()) return;

    // Avoid adding a zero-length line.
    if (!contours.back().points.empty()) {
        if (contours.back().points.back() == Vec2<float>(x, y)) {
            return;
        }
    }

    auto &current_contour = contours.back();

    current_contour.points.emplace_back(x, y);
    current_contour.flags.emplace_back();

    // Update contour bounds.
    union_rect(current_contour.bounds, Vec2<float>(x, y));

    // Update path bounds.
    bounds = bounds.union_rect(current_contour.bounds);
}

void Outline::curve_to(float cx, float cy, float x, float y) {
    if (contours.empty()) return;

    auto &current_contour = contours.back();

    current_contour.points.emplace_back(cx, cy);
    current_contour.flags.emplace_back(CONTROL_POINT_0);

    current_contour.points.emplace_back(x, y);
    current_contour.flags.emplace_back();

    // Update contour bounds.
    union_rect(current_contour.bounds, Vec2<float>(cx, cy));
    union_rect(current_contour.bounds, Vec2<float>(x, y));

    // Update path bounds.
    bounds = bounds.union_rect(current_contour.bounds);
}

void Outline::cubic_to(float cx, float cy, float cx1, float cy1, float x, float y) {
    if (contours.empty()) return;

    auto &current_contour = contours.back();

    current_contour.points.emplace_back(cx, cy);
    current_contour.flags.emplace_back(CONTROL_POINT_0);

    current_contour.points.emplace_back(cx1, cy1);
    current_contour.flags.emplace_back(CONTROL_POINT_1);

    current_contour.points.emplace_back(x, y);
    current_contour.flags.emplace_back();

    // Update contour bounds.
    union_rect(current_contour.bounds, Vec2<float>(cx, cy));
    union_rect(current_contour.bounds, Vec2<float>(cx1, cy1));
    union_rect(current_contour.bounds, Vec2<float>(x, y));

    // Update path bounds.
    bounds = bounds.union_rect(current_contour.bounds);
}

void Outline::close() {
    if (contours.empty()) return;

    auto &current_contour = contours.back();

    // If the last segment is a line and the last point is the same as the first point in a close contour, just ignore
    // the last one. This is to make sure the contour will be correctly stroked.
    // FIXME: This is a makeshift fix for text stroke rendering.
    if (current_contour.points.size() > 1) {
        if ((current_contour.flags.rbegin() + 1)->value == 0) {
            if (current_contour.points.back() == current_contour.points.front()) {
                current_contour.points.pop_back();
                current_contour.flags.pop_back();
            }
        }
    }

    current_contour.closed = true;
}

void Outline::add_line(Vec2<float> p_start, Vec2<float> p_end) {
    if (p_start == p_end) return;

    move_to(p_start.x, p_start.y);
    line_to(p_end.x, p_end.y);
}

void Outline::add_rect(const Rect<float> &p_rect, float p_corner_radius) {
    if (p_rect.size().x == 0 || p_rect.size().y == 0) return;

    if (p_corner_radius <= 0) {
        move_to(p_rect.min_x(), p_rect.min_y());
        line_to(p_rect.max_x(), p_rect.min_y());
        line_to(p_rect.max_x(), p_rect.max_y());
        line_to(p_rect.min_x(), p_rect.max_y());
        close();

        return;
    }

    // Corner radius can't be greater than the half of the shorter line of the rect.
    p_corner_radius = std::min(p_corner_radius, std::min(p_rect.width(), p_rect.height()) * 0.5f);

    // See https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves.
    float adjusted_radius = p_corner_radius * RATIO;

    move_to(p_rect.min_x(), p_rect.min_y() + p_corner_radius);
    cubic_to(p_rect.min_x(),
             p_rect.min_y() + p_corner_radius - adjusted_radius,
             p_rect.min_x() + p_corner_radius - adjusted_radius,
             p_rect.min_y(),
             p_rect.min_x() + p_corner_radius,
             p_rect.min_y());
    line_to(p_rect.max_x() - p_corner_radius, p_rect.min_y());
    cubic_to(p_rect.max_x() - p_corner_radius + adjusted_radius,
             p_rect.min_y(),
             p_rect.max_x(),
             p_rect.min_y() + p_corner_radius - adjusted_radius,
             p_rect.max_x(),
             p_rect.min_y() + p_corner_radius);
    line_to(p_rect.max_x(), p_rect.max_y() - p_corner_radius);
    cubic_to(p_rect.max_x(),
             p_rect.max_y() - p_corner_radius + adjusted_radius,
             p_rect.max_x() - p_corner_radius + adjusted_radius,
             p_rect.max_y(),
             p_rect.max_x() - p_corner_radius,
             p_rect.max_y());
    line_to(p_rect.min_x() + p_corner_radius, p_rect.max_y());
    cubic_to(p_rect.min_x() + p_corner_radius - adjusted_radius,
             p_rect.max_y(),
             p_rect.min_x(),
             p_rect.max_y() - p_corner_radius + adjusted_radius,
             p_rect.min_x(),
             p_rect.max_y() - p_corner_radius);
    close();
}

void Outline::add_circle(Vec2<float> p_center, float p_radius) {
    if (p_radius == 0) return;

    // See https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves.
    float adjusted_radius = p_radius * RATIO;

    move_to(p_center.x, p_center.y - p_radius);
    cubic_to(p_center.x + adjusted_radius,
             p_center.y - p_radius,
             p_center.x + p_radius,
             p_center.y - adjusted_radius,
             p_center.x + p_radius,
             p_center.y);
    cubic_to(p_center.x + p_radius,
             p_center.y + adjusted_radius,
             p_center.x + adjusted_radius,
             p_center.y + p_radius,
             p_center.x,
             p_center.y + p_radius);
    cubic_to(p_center.x - adjusted_radius,
             p_center.y + p_radius,
             p_center.x - p_radius,
             p_center.y + adjusted_radius,
             p_center.x - p_radius,
             p_center.y);
    cubic_to(p_center.x - p_radius,
             p_center.y - adjusted_radius,
             p_center.x - adjusted_radius,
             p_center.y - p_radius,
             p_center.x,
             p_center.y - p_radius);
    close();
}

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
