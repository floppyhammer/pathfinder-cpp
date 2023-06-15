#include "path2d.h"

#include "../common/math/basic.h"

namespace Pathfinder {

void Path2d::close_path() {
    current_contour.close();
}

void Path2d::move_to(float x, float y) {
    flush_current_contour();
    current_contour.push_endpoint({x, y});
}

void Path2d::line_to(float x, float y) {
    current_contour.push_endpoint({x, y});
}

void Path2d::quadratic_to(float cx, float cy, float x, float y) {
    Vec2F &point0 = current_contour.points.back();
    Vec2F ctrl = {cx, cy};
    Vec2F point1 = {x, y};

    // Degenerated into a line.
    if (point0.approx_eq(ctrl, FLOAT_EPSILON) || point1.approx_eq(ctrl, FLOAT_EPSILON)) {
        current_contour.push_endpoint(point1);
    }

    current_contour.push_quadratic(ctrl, point1);
}

void Path2d::cubic_to(float cx0, float cy0, float cx1, float cy1, float x, float y) {
    Vec2F &point0 = current_contour.points.back();
    Vec2F ctrl0 = {cx0, cy0};
    Vec2F ctrl1 = {cx1, cy1};
    Vec2F point1 = {x, y};

    // The degeneration into lower-level curves are to fix incorrect line caps.
    if (ctrl0.approx_eq(ctrl1, FLOAT_EPSILON) && ctrl1.approx_eq(point1, FLOAT_EPSILON)) {
        // No point to add the exactly same on-curve and control points.
        if (point0.approx_eq(ctrl0, FLOAT_EPSILON)) {
            return;
        }

        // Degenerated into a line.
        current_contour.push_endpoint(point1);
    }

    // Degenerated into a quadratic curve.
    if (point0.approx_eq(ctrl0, FLOAT_EPSILON)) {
        current_contour.push_quadratic(ctrl1, point1);
        return;
    }
    if (point1.approx_eq(ctrl1, FLOAT_EPSILON)) {
        current_contour.push_quadratic(ctrl0, point1);
        return;
    }

    current_contour.push_cubic({cx0, cy0}, {cx1, cy1}, {x, y});
}

void Path2d::add_line(const Vec2F &start, const Vec2F &end) {
    if (start == end) {
        return;
    }

    move_to(start.x, start.y);
    line_to(end.x, end.y);
}

const float CIRCLE_RATIO = 0.552284749831; // 4.0f * (sqrt(2.0f) - 1.0f) / 3.0f

void Path2d::add_rect(const RectF &rect, float corner_radius) {
    if (rect.size().x == 0 || rect.size().y == 0) {
        return;
    }

    if (corner_radius <= 0) {
        move_to(rect.min_x(), rect.min_y());
        line_to(rect.max_x(), rect.min_y());
        line_to(rect.max_x(), rect.max_y());
        line_to(rect.min_x(), rect.max_y());
        close_path();

        return;
    }

    // Corner radius can't be greater than the half of the shorter line of the rect.
    corner_radius = std::min(corner_radius, std::min(rect.width(), rect.height()) * 0.5f);

    // See https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves.
    float adjusted_radius = corner_radius * CIRCLE_RATIO;

    move_to(rect.min_x(), rect.min_y() + corner_radius);
    cubic_to(rect.min_x(),
             rect.min_y() + corner_radius - adjusted_radius,
             rect.min_x() + corner_radius - adjusted_radius,
             rect.min_y(),
             rect.min_x() + corner_radius,
             rect.min_y());
    line_to(rect.max_x() - corner_radius, rect.min_y());
    cubic_to(rect.max_x() - corner_radius + adjusted_radius,
             rect.min_y(),
             rect.max_x(),
             rect.min_y() + corner_radius - adjusted_radius,
             rect.max_x(),
             rect.min_y() + corner_radius);
    line_to(rect.max_x(), rect.max_y() - corner_radius);
    cubic_to(rect.max_x(),
             rect.max_y() - corner_radius + adjusted_radius,
             rect.max_x() - corner_radius + adjusted_radius,
             rect.max_y(),
             rect.max_x() - corner_radius,
             rect.max_y());
    line_to(rect.min_x() + corner_radius, rect.max_y());
    cubic_to(rect.min_x() + corner_radius - adjusted_radius,
             rect.max_y(),
             rect.min_x(),
             rect.max_y() - corner_radius + adjusted_radius,
             rect.min_x(),
             rect.max_y() - corner_radius);
    close_path();
}

void Path2d::add_circle(const Vec2F &center, float radius) {
    if (radius == 0) {
        return;
    }

    // See https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves.
    float adjusted_radius = radius * CIRCLE_RATIO;

    move_to(center.x, center.y - radius);
    cubic_to(center.x + adjusted_radius,
             center.y - radius,
             center.x + radius,
             center.y - adjusted_radius,
             center.x + radius,
             center.y);
    cubic_to(center.x + radius,
             center.y + adjusted_radius,
             center.x + adjusted_radius,
             center.y + radius,
             center.x,
             center.y + radius);
    cubic_to(center.x - adjusted_radius,
             center.y + radius,
             center.x - radius,
             center.y + adjusted_radius,
             center.x - radius,
             center.y);
    cubic_to(center.x - radius,
             center.y - adjusted_radius,
             center.x - adjusted_radius,
             center.y - radius,
             center.x,
             center.y - radius);
    close_path();
}

Outline Path2d::into_outline() {
    flush_current_contour();
    return outline;
}

void Path2d::flush_current_contour() {
    if (!current_contour.is_empty()) {
        outline.push_contour(current_contour);
        current_contour = Contour();
    }
}

} // namespace Pathfinder
