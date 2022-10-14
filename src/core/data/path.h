#ifndef PATHFINDER_PATH_H
#define PATHFINDER_PATH_H

#include <vector>

#include "../../common/color.h"
#include "contour.h"

namespace Pathfinder {

/// A vector path to be filled (without drawing info).
/// In order to differentiate it from DrawPath and ClipPath, we use the name 'Outline' here.
/// Outlines consist of contours (a.k.a. sub-paths).
class Outline {
public:
    std::vector<Contour> contours;

    /// Bounding box.
    Rect<float> bounds;

public:
    /**
     * Translate the SVG image. Bounds are also updated.
     * @param translation The translation vector.
     */
    void translate(const Vec2<float> &translation);

    /**
     * Scale the SVG image. Bounds are also updated.
     * @param scale The scaling factor.
     */
    void scale(const Vec2<float> &scale);

    /**
     * Rotate the SVG image. Bounds are also updated.
     * @param rotation The rotation angle in degree.
     */
    void rotate(float rotation);

    /// Applies an affine transform to this shape and all its paths.
    void transform(const Transform2 &transform);

    /**
     * Update shape and paths bounds. Required after transforming shape.
     */
    void update_bounds();

    /// Add a new contour to this shape.
    void push_contour(const Contour &p_contour);
};

/// A thin wrapper over Outline, which describes a path that can be drawn.
struct DrawPath {
    /// The actual vector path.
    Outline outline;

    /// The ID of the paint that specifies how to fill this shape.
    uint16_t paint{};

    /// The ID of an optional clip shape that will be used to clip this shape.
    uint32_t clip_path{};

    /// How to fill this shape (winding or even-odd).
    FillRule fill_rule = FillRule::Winding;

    /// How to blend this shape with everything below it.
    BlendMode blend_mode{};
};

/// A thin wrapper over Outline, which describes a path that can be used to clip other paths.
struct ClipPath {
    /// The actual vector path.
    Outline outline;

    /// The ID of another, previously-defined, clip shape that clips this one.
    /// Nested clips can be achieved by clipping clip shapes with other clip shapes.
    uint32_t clip_path{};

    /// How to fill this shape (winding or even-odd).
    FillRule fill_rule = FillRule::Winding;
};

} // namespace Pathfinder

#endif // PATHFINDER_PATH_H
