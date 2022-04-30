#ifndef PATHFINDER_API_H
#define PATHFINDER_API_H

/*************************************************************************
 * Pathfinder API
 *************************************************************************/

namespace PathfinderApi {
    struct ColorU;

    /// Fill rule for a shape.
    enum class FillRule {
        /// The nonzero rule: https://en.wikipedia.org/wiki/Nonzero-rule
        Winding,
        /// The even-odd rule: https://en.wikipedia.org/wiki/Even%E2%80%93odd_rule
        EvenOdd,
    };

    /// The shape of the ends of the stroke.
    enum class LineCap {
        /// The ends of lines are squared off at the endpoints.
        Butt,
        /// The ends of lines are squared off by adding a box with an equal width and half the height
        /// of the line's thickness.
        Square,
        /// The ends of lines are rounded.
        Round,
    };

    /// The shape used to join two line segments where they meet.
    enum class LineJoin {
        /// Connected segments are joined by extending their outside edges to connect at a single
        /// point, with the effect of filling an additional lozenge-shaped area. The `f32` value
        /// specifies the miter limit ratio.
        Miter,
        /// Fills an additional triangular area between the common endpoint of connected segments and
        /// the separate outside rectangular corners of each segment.
        Bevel,
        /// Rounds off the corners of a shape by filling an additional sector of disc centered at the
        /// common endpoint of connected segments. The radius for these rounded corners is equal to the
        /// line width.
        Round,
    };

    /// Defines how a shape is to be filled: with a solid color, gradient, or pattern.
    class Paint {
    public:
        /// Creates a simple paint from a single base color.
        virtual void from_color(const ColorU &color) = 0;

        /// Creates a paint from a gradient.
        //virtual Paint from_gradient(const Gradient &gradient) = 0;

        /// Creates a paint from a raster pattern.
        //virtual Paint from_pattern(const Pattern& pattern) = 0;
    };

    class Shape {
    public:
        /// Basic geometries.
        /// -----------------------------------------------
        virtual void move_to(float x, float y) = 0;

        virtual void line_to(float x, float y) = 0;

        virtual void curve_to(float cx, float cy, float x, float y) = 0;

        virtual void cubic_to(float cx, float cy, float cx1, float cy1, float x, float y) = 0;

        virtual void close() = 0;
        /// -----------------------------------------------

        /// Advanced geometries.
        /// -----------------------------------------------
        virtual void add_line(float start_x, float start_y, float end_x, float end_y) = 0;

        virtual void add_rect(float left, float top, float right, float bottom, float corner_radius) = 0;

        virtual void add_circle(float center_x, float center_y, float radius) = 0;
        /// -----------------------------------------------
    };

    class Canvas {
    public:
//        Canvas(const std::shared_ptr<Driver> &p_driver, float p_size_x, float p_size_y,
//                     const std::vector<char> &area_lut_input);

        // Set state.
        // ------------------------------------------------
        virtual void set_fill_paint(const Paint &paint) = 0;

        virtual void set_stroke_paint(const Paint &paint) = 0;

        virtual void set_line_width(float p_line_width) = 0;

        virtual void set_line_cap(LineCap cap) = 0;

        virtual void set_line_join(LineJoin join) = 0;

        virtual void set_miter_limit(float p_miter_limit) = 0;

        virtual void set_shadow_blur(float p_shadow_blur) = 0;

        virtual void set_shadow_color(const ColorU &color) = 0;

        virtual void set_shadow_offset(float offset_x, float offset_y) = 0;

        virtual void set_line_dash(const std::vector<float> &dash) = 0;

        virtual void set_line_dash_offset(float offset) = 0;
        // ------------------------------------------------

        // Drawing ops.
        // ------------------------------------------------
        /// Fill a shape.
        virtual void fill_shape(Shape p_shape, FillRule p_fill_rule) = 0;

        /// Stroke a shape.
        virtual void stroke_shape(Shape p_shape) = 0;
        // ------------------------------------------------

        /// Rebuild and redraw the scene.
        virtual void draw() = 0;

        /// Clear the scene.
        virtual void clear() = 0;

        //std::shared_ptr<Texture> get_dest_texture();

        /**
         * @brief Load a SVG file into cubic Bezier curves.
         * @param canvas Target canvas.
         * @param input SVG file content.
         */
        virtual void load_svg(const std::string &input) = 0;
    };
}

#endif //PATHFINDER_API_H
