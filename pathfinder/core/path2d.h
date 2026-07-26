#pragma once

#include "../common/math/rect.h"
#include "../common/math/vec2.h"
#include "data/path.h"

namespace Pathfinder {

/**
 * @brief Represents a 2D path consisting of multiple contours.
 * @note This structure does NOT strictly reflect the original input data as it automatically
 * performs point deduplication and simplifies degenerate segments during construction for
 * numerical stability.
 */
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

    void transform(const Transform2 &transform);

    /**
     * @brief Get SVG path data string (the 'd' attribute).
     * @return SVG path data.
     */
    std::string to_svg_path_data();

    /**
     * @brief Get a full SVG string containing this path.
     * @return Full SVG string.
     */
    std::string to_svg_string();

private:
    Contour current_contour;

    Outline outline;

    void flush_current_contour();
};

} // namespace Pathfinder
