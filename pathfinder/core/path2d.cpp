#include "path2d.h"

#include <sstream>

#include "../common/math/basic.h"

namespace Pathfinder {

constexpr float PATH_MIN_DIST = 0.05f;
constexpr float PATH_MIN_DIST_SQ = PATH_MIN_DIST * PATH_MIN_DIST;

void Path2d::close_path() {
    current_contour.close();
}

void Path2d::move_to(float x, float y) {
    flush_current_contour();
    current_contour.push_endpoint({x, y});
}

void Path2d::line_to(float x, float y) {
    Vec2F to = {x, y};
    if (!current_contour.points.empty()) {
        if ((to - current_contour.points.back()).square_length() < PATH_MIN_DIST_SQ) {
            return;
        }
    }
    current_contour.push_endpoint(to);
}

void Path2d::quadratic_to(float cx, float cy, float x, float y) {
    Vec2F &point0 = current_contour.points.back();
    Vec2F ctrl = {cx, cy};
    Vec2F point1 = {x, y};

    // Degenerates into a line.
    if (point0.approx_eq(ctrl, PATH_MIN_DIST) || point1.approx_eq(ctrl, PATH_MIN_DIST)) {
        line_to(x, y);
        return;
    }

    current_contour.push_quadratic(ctrl, point1);
}

void Path2d::cubic_to(float cx0, float cy0, float cx1, float cy1, float x, float y) {
    Vec2F &point0 = current_contour.points.back();
    Vec2F ctrl0 = {cx0, cy0};
    Vec2F ctrl1 = {cx1, cy1};
    Vec2F point1 = {x, y};

    // Degenerates into a line.
    if (ctrl0.approx_eq(ctrl1, PATH_MIN_DIST) && ctrl1.approx_eq(point1, PATH_MIN_DIST)) {
        line_to(x, y);
        return;
    }

    current_contour.push_cubic({cx0, cy0}, {cx1, cy1}, {x, y});
}

void Path2d::add_line(const Vec2F &start, const Vec2F &end) {
    if (start.approx_eq(end, FLOAT_EPSILON)) {
        return;
    }

    move_to(start.x, start.y);
    line_to(end.x, end.y);
}

constexpr float CIRCLE_RATIO = 0.552284749831; // 4.0f * (sqrt(2.0f) - 1.0f) / 3.0f

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

    add_rect_with_corners(rect, RectF(corner_radius, corner_radius, corner_radius, corner_radius));
}

void Path2d::add_rect_with_corners(const RectF &rect, const RectF &corner_radius) {
    if (rect.size().x == 0 || rect.size().y == 0) {
        return;
    }

    // Corner radius can't be greater than the half of the shorter line of the rect.
    float top_left = std::min(corner_radius.left, std::min(rect.width(), rect.height()) * 0.5f);
    float top_right = std::min(corner_radius.top, std::min(rect.width(), rect.height()) * 0.5f);
    float bottom_left = std::min(corner_radius.right, std::min(rect.width(), rect.height()) * 0.5f);
    float bottom_right = std::min(corner_radius.bottom, std::min(rect.width(), rect.height()) * 0.5f);

    // See https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves.
    float adjusted_top_left = top_left * CIRCLE_RATIO;
    float adjusted_top_right = top_right * CIRCLE_RATIO;
    float adjusted_bottom_left = bottom_left * CIRCLE_RATIO;
    float adjusted_bottom_right = bottom_right * CIRCLE_RATIO;

    move_to(rect.min_x(), rect.min_y() + top_left);
    cubic_to(rect.min_x(),
             rect.min_y() + top_left - adjusted_top_left,
             rect.min_x() + top_left - adjusted_top_left,
             rect.min_y(),
             rect.min_x() + top_left,
             rect.min_y());
    line_to(rect.max_x() - top_right, rect.min_y());
    cubic_to(rect.max_x() - top_right + adjusted_top_right,
             rect.min_y(),
             rect.max_x(),
             rect.min_y() + top_right - adjusted_top_right,
             rect.max_x(),
             rect.min_y() + top_right);
    line_to(rect.max_x(), rect.max_y() - bottom_left);
    cubic_to(rect.max_x(),
             rect.max_y() - bottom_left + adjusted_bottom_left,
             rect.max_x() - bottom_left + adjusted_bottom_left,
             rect.max_y(),
             rect.max_x() - bottom_left,
             rect.max_y());
    line_to(rect.min_x() + bottom_right, rect.max_y());
    cubic_to(rect.min_x() + bottom_right - adjusted_bottom_right,
             rect.max_y(),
             rect.min_x(),
             rect.max_y() - bottom_right + adjusted_bottom_right,
             rect.min_x(),
             rect.max_y() - bottom_right);
    close_path();
}

void Path2d::add_circle(const Vec2F &center, float radius) {
    if (radius <= 0) {
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

void Path2d::add_path(const Path2d &other, const Transform2 &transform) {
    flush_current_contour();

    // We need to get the outline from the other path.
    // Since into_outline() might be destructive/mutating, we'll use a const_cast
    // or better, just access the internal outline if we are a member.
    Outline other_outline = const_cast<Path2d &>(other).into_outline();
    other_outline.transform(transform);

    for (const auto &contour : other_outline.contours) {
        outline.push_contour(contour);
    }
}

Outline Path2d::into_outline() {
    flush_current_contour();
    return outline;
}

void Path2d::transform(const Transform2 &transform) {
    // Make sure to flush current contour into outline before applying transform.
    flush_current_contour();
    outline.transform(transform);
}

void Path2d::flush_current_contour() {
    if (!current_contour.is_empty()) {
        outline.push_contour(current_contour);
        current_contour = Contour();
    }
}

std::string Path2d::to_svg_path_data() {
    std::ostringstream oss;

    auto append_contour = [&](const Contour &contour) {
        if (contour.is_empty()) {
            return;
        }

        auto segments = contour.get_segments(false);

        if (segments.empty()) {
            return;
        }

        // Start point.
        oss << "M " << segments[0].baseline.from().x << " " << segments[0].baseline.from().y << " ";

        for (const auto &segment : segments) {
            switch (segment.kind) {
                case SegmentKind::Line:
                    oss << "L " << segment.baseline.to().x << " " << segment.baseline.to().y << " ";
                    break;
                case SegmentKind::Quadratic:
                    oss << "Q " << segment.ctrl.from().x << " " << segment.ctrl.from().y << " "
                        << segment.baseline.to().x << " " << segment.baseline.to().y << " ";
                    break;
                case SegmentKind::Cubic:
                    oss << "C " << segment.ctrl.from().x << " " << segment.ctrl.from().y << " " << segment.ctrl.to().x
                        << " " << segment.ctrl.to().y << " " << segment.baseline.to().x << " "
                        << segment.baseline.to().y << " ";
                    break;
                default:
                    break;
            }
        }

        if (contour.closed) {
            oss << "Z ";
        }
    };

    // Append finished contours.
    for (const auto &contour : outline.contours) {
        append_contour(contour);
    }

    // Append active contour without flushing.
    append_contour(current_contour);

    return oss.str();
}

std::string Path2d::to_svg_string() {
    auto path_data = to_svg_path_data();

    auto b = outline.bounds;
    if (!current_contour.is_empty()) {
        b = b.union_rect(current_contour.bounds);
    }

    if (!b.is_valid()) {
        b = RectF(0, 0, 0, 0);
    }

    std::ostringstream oss;

    oss << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"" << b.min_x() << " " << b.min_y() << " " << b.width()
        << " " << b.height() << "\">";
    oss << "<path d=\"" << path_data << "\" fill=\"black\" />";
    oss << "</svg>";

    return oss.str();
}

} // namespace Pathfinder
