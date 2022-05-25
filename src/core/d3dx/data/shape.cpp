#include "shape.h"

#include "../../../common/math/basic.h"

namespace Pathfinder {
    const float RATIO = 0.552284749831; // 4.0f * (sqrt(2.0f) - 1.0f) / 3.0f

    void Shape::move_to(float x, float y) {
        // Add a new empty path.
        paths.emplace_back();

        auto &current_path = paths.back();

        // First on-curve point.
        paths.back().points.emplace_back(x, y);
        paths.back().flags.emplace_back();

        // Update path bounds.
        union_rect(current_path.bounds, Vec2<float>(x, y), true);

        // Update shape bounds.
        bounds = bounds.union_rect(current_path.bounds);
    }

    void Shape::line_to(float x, float y) {
        if (paths.empty())
            return;

        // Avoid adding a zero-length line.
        if (!paths.back().points.empty()) {
            if (paths.back().points.back() == Vec2<float>(x, y)) {
                return;
            }
        }

        auto &current_path = paths.back();

        current_path.points.emplace_back(x, y);
        current_path.flags.emplace_back();

        // Update path bounds.
        union_rect(current_path.bounds, Vec2<float>(x, y));

        // Update shape bounds.
        bounds = bounds.union_rect(current_path.bounds);
    }

    void Shape::curve_to(float cx, float cy, float x, float y) {
        if (paths.empty())
            return;

        auto &current_path = paths.back();

        current_path.points.emplace_back(cx, cy);
        current_path.flags.emplace_back(PointFlags::CONTROL_POINT_0);

        current_path.points.emplace_back(x, y);
        current_path.flags.emplace_back();

        // Update path bounds.
        union_rect(current_path.bounds, Vec2<float>(cx, cy));
        union_rect(current_path.bounds, Vec2<float>(x, y));

        // Update shape bounds.
        bounds = bounds.union_rect(current_path.bounds);
    }

    void Shape::cubic_to(float cx, float cy, float cx1, float cy1, float x, float y) {
        if (paths.empty())
            return;

        auto &current_path = paths.back();

        current_path.points.emplace_back(cx, cy);
        current_path.flags.emplace_back(PointFlags::CONTROL_POINT_0);

        current_path.points.emplace_back(cx1, cy1);
        current_path.flags.emplace_back(PointFlags::CONTROL_POINT_1);

        current_path.points.emplace_back(x, y);
        current_path.flags.emplace_back();

        // Update path bounds.
        union_rect(current_path.bounds, Vec2<float>(cx, cy));
        union_rect(current_path.bounds, Vec2<float>(cx1, cy1));
        union_rect(current_path.bounds, Vec2<float>(x, y));

        // Update shape bounds.
        bounds = bounds.union_rect(current_path.bounds);
    }

    void Shape::close() {
        if (paths.empty())
            return;

        auto &current_path = paths.back();

        // If the last segment is a line and the last point is the same as the first point in a close path, just ignore the last one.
        // This is to make sure the path will be correctly stroked.
        // FIXME: This is a makeshift fix for text stroke rendering.
        if (current_path.points.size() > 1) {
            if ((current_path.flags.rbegin() + 1)->value == 0) {
                if (current_path.points.back() == current_path.points.front()) {
                    current_path.points.pop_back();
                    current_path.flags.pop_back();
                }
            }
        }

        current_path.closed = true;
    }

    void Shape::add_line(Vec2<float> p_start, Vec2<float> p_end) {
        if (p_start == p_end) return;

        move_to(p_start.x, p_start.y);
        line_to(p_end.x, p_end.y);
    }

    void Shape::add_rect(const Rect<float> &p_rect, float p_corner_radius) {
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
        cubic_to(p_rect.min_x(), p_rect.min_y() + p_corner_radius - adjusted_radius,
                 p_rect.min_x() + p_corner_radius - adjusted_radius, p_rect.min_y(), p_rect.min_x() + p_corner_radius,
                 p_rect.min_y());
        line_to(p_rect.max_x() - p_corner_radius, p_rect.min_y());
        cubic_to(p_rect.max_x() - p_corner_radius + adjusted_radius, p_rect.min_y(), p_rect.max_x(),
                 p_rect.min_y() + p_corner_radius - adjusted_radius, p_rect.max_x(), p_rect.min_y() + p_corner_radius);
        line_to(p_rect.max_x(), p_rect.max_y() - p_corner_radius);
        cubic_to(p_rect.max_x(), p_rect.max_y() - p_corner_radius + adjusted_radius,
                 p_rect.max_x() - p_corner_radius + adjusted_radius, p_rect.max_y(), p_rect.max_x() - p_corner_radius,
                 p_rect.max_y());
        line_to(p_rect.min_x() + p_corner_radius, p_rect.max_y());
        cubic_to(p_rect.min_x() + p_corner_radius - adjusted_radius, p_rect.max_y(), p_rect.min_x(),
                 p_rect.max_y() - p_corner_radius + adjusted_radius, p_rect.min_x(), p_rect.max_y() - p_corner_radius);
        close();
    }

    void Shape::add_circle(Vec2<float> p_center, float p_radius) {
        if (p_radius == 0) return;

        // See https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves.
        float adjusted_radius = p_radius * RATIO;

        move_to(p_center.x, p_center.y - p_radius);
        cubic_to(p_center.x + adjusted_radius, p_center.y - p_radius, p_center.x + p_radius,
                 p_center.y - adjusted_radius, p_center.x + p_radius, p_center.y);
        cubic_to(p_center.x + p_radius, p_center.y + adjusted_radius, p_center.x + adjusted_radius,
                 p_center.y + p_radius, p_center.x, p_center.y + p_radius);
        cubic_to(p_center.x - adjusted_radius, p_center.y + p_radius, p_center.x - p_radius,
                 p_center.y + adjusted_radius, p_center.x - p_radius, p_center.y);
        cubic_to(p_center.x - p_radius, p_center.y - adjusted_radius, p_center.x - adjusted_radius,
                 p_center.y - p_radius, p_center.x, p_center.y - p_radius);
        close();
    }

    void Shape::scale(const Vec2<float> &scale) {
        // Shape origin.
        //Vec2<float> origin = bounds.origin();
        Vec2<float> origin{};

        for (auto &path: paths) {
            for (auto &point: path.points) {
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

    void Shape::translate(const Vec2<float> &translation) {
        for (auto &path: paths) {
            for (auto &point: path.points) {
                point += translation;
            }
        }

        update_bounds();
    }

    void Shape::rotate(float rotation) {
        // Shape origin.
        //Vec2<float> origin = bounds.origin();
        Vec2<float> origin{};

        float rotation_in_radian = deg2rad(rotation);

        for (auto &path: paths) {
            for (auto &point: path.points) {
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

    void Shape::transform(const Transform2 &transform) {
        for (auto &path: paths) {
            for (auto &point: path.points) {
                point = transform * point;
            }
        }

        update_bounds();
    }

    void Shape::update_bounds() {
        bounds = Rect<float>();

        // Update child paths' bounds and its own bounds.
        for (auto &path: paths) {
            path.bounds = Rect<float>();

            for (auto &point: path.points) {
                union_rect(path.bounds, point);
            }

            // Own bounds.
            bounds = bounds.union_rect(path.bounds);
        }
    }

    void Shape::push_path(const Path &p_path) {
        if (p_path.is_empty()) {
            return;
        }

        // Push path.
        paths.push_back(p_path);

        // Update bounds.
        if (paths.empty()) {
            bounds = p_path.bounds;
        } else {
            bounds = bounds.union_rect(p_path.bounds);
        }
    }
}
