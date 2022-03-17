//
// Created by chy on 7/9/2021.
//

#ifndef PATHFINDER_SHAPE_H
#define PATHFINDER_SHAPE_H

#include "path.h"
#include "../../../common/color.h"

#include <vector>

namespace Pathfinder {
/// A shape drawn to the output or to a render target.
/// Same as DrawPath + Outline in Pathfinder Rust.
    struct Shape {
    public:
        /// Paths (each path starts with the MoveTo command).
        std::vector<Path> paths;

        /// Bounding box.
        Rect<float> bounds;

        /// The ID of the paint that specifies how to fill this shape.
        uint16_t paint;

        /// The ID of an optional clip shape that will be used to clip this shape.
        uint32_t clip_shape;

        /// How to fill this path (winding (nonzero) or even-odd).
        FillRule fill_rule = FillRule::Winding;

        /// Post-processing blur of the whole shape.
        float blur = 0;

        /// How to blend this shape with everything below it.
        BlendMode blend_mode{};

        /// Basic geometries.
        /// -----------------------------------------------
        void move_to(float x, float y);

        void line_to(float x, float y);

        void curve_to(float cx, float cy, float x, float y);

        void cubic_to(float cx, float cy, float cx1, float cy1, float x, float y);

        void close();
        /// -----------------------------------------------

        /// Advanced geometries.
        /// -----------------------------------------------
        void add_line(Vec2<float> p_start, Vec2<float> p_end);

        void add_rect(const Rect<float> &p_rect, float p_corner_radius = 0);

        /// There is no exact representation of the circle using Bezier curves.
        void add_circle(Vec2<float> p_center, float p_radius);
        /// -----------------------------------------------

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

        /// Add a new path to this shape.
        void push_path(const Path &p_path);
    };

/// Describes a shape that can be used to clip other shapes.
    struct ClipShape {
        /// The actual vector paths.
        std::vector<Path> paths;

        /// Bounding box.
        Rect<float> bounds;

        /// The ID of another, previously-defined, clip shape that clips this one.
        /// Nested clips can be achieved by clipping clip shapes with other clip shapes.
        uint32_t clip_shape;

        /// How to fill this shape (winding or even-odd).
        FillRule fill_rule = FillRule::Winding;
    };
}

#endif //PATHFINDER_SHAPE_H
