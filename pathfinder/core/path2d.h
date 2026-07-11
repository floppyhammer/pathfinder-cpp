#pragma once

#include "../common/math/rect.h"
#include "../common/math/vec2.h"
#include "data/path.h"

namespace Pathfinder {

class Path2d {
public:
    // Basic geometries.
    // -----------------------------------------------
    void close_path();

    void move_to(float x, float y);

    void line_to(float x, float y);

    void quadratic_to(float cx, float cy, float x, float y);

    void cubic_to(float cx, float cy, float cx1, float cy1, float x, float y);
    // -----------------------------------------------

    // Advanced geometries.
    // -----------------------------------------------
    void add_line(const Vec2F &start, const Vec2F &end);

    void add_rect(const RectF &rect, float corner_radius = 0);

    void add_rect_with_corners(const RectF &rect, const RectF &corner_radius);

    void add_circle(const Vec2F &center, float radius);

    void add_path(const Path2d &other, const Transform2 &transform = Transform2::from_scale({1.0f, 1.0f}));
    // -----------------------------------------------

    /// Returns the outline.
    Outline into_outline();

private:
    Contour current_contour;

    Outline outline;

    void flush_current_contour();
};

} // namespace Pathfinder
