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
    RectF bounds;

public:
    /// Applies an affine transform to this shape and all its paths.
    void transform(const Transform2 &transform);

    /// Add a new contour to this shape.
    void push_contour(const Contour &_contour);
};

/// A thin wrapper over Outline, which describes a path that can be drawn.
struct DrawPath {
    /// The actual vector path.
    Outline outline;

    /// The ID of the paint that specifies how to fill this shape.
    uint16_t paint{};

    /// The ID of an optional clip shape that will be used to clip this shape.
    std::shared_ptr<uint32_t> clip_path; // Optional

    /// How to fill this shape (winding or even-odd).
    FillRule fill_rule = FillRule::Winding;

    /// How to blend this shape with everything below it.
    BlendMode blend_mode{};
};

/// A thin wrapper over Outline, which describes a path that can be used to clip other paths.
struct ClipPath {
    /// The actual vector path.
    Outline outline;

    /// The ID of another, previously-defined, clip path that clips this one.
    ///
    /// Nested clips can be achieved by clipping clip paths with other clip paths.
    std::shared_ptr<uint32_t> clip_path; // Optional

    /// How to fill this shape (winding or even-odd).
    FillRule fill_rule = FillRule::Winding;
};

} // namespace Pathfinder

#endif // PATHFINDER_PATH_H
