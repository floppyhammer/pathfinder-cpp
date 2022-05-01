#ifndef PATHFINDER_CANVAS_H
#define PATHFINDER_CANVAS_H

#include "../../gpu/driver.h"
#include "../d3d9/scene_builder.h"
#include "../d3d9/renderer.h"
#include "../d3d11/scene_builder.h"
#include "../d3d11/renderer.h"

#include <memory>

namespace Pathfinder {
    enum ShapeOp {
        Fill,
        Stroke,
    };

    enum CompositeOperation {
        SourceOver,
        SourceIn,
        SourceOut,
        SourceAtop,
        DestinationOver,
        DestinationIn,
        DestinationOut,
        DestinationAtop,
        Lighter,
        Copy,
        Xor,
        Multiply,
        Screen,
        Overlay,
        Darken,
        Lighten,
        ColorDodge,
        ColorBurn,
        HardLight,
        SoftLight,
        Difference,
        Exclusion,
        Hue,
        Saturation,
        Color,
        Luminosity,
    };

    /// Pen state.
    struct State {
        Transform2 transform;
        float line_width = 0;

        LineCap line_cap = LineCap::Butt;
        LineJoin line_join = LineJoin::Miter;
        float miter_limit = 10;

        std::vector<float> line_dash;
        float line_dash_offset = 0;

        Paint fill_paint;
        Paint stroke_paint;

        ColorU shadow_color;
        float shadow_blur = 0;
        Vec2<float> shadow_offset;

        BlendMode global_composite_operation;
    };

    class Canvas {
    public:
        Canvas(const std::shared_ptr<Driver>& p_driver, float p_size_x, float p_size_y, const std::vector<char> &area_lut_input);

        // Set state.
        // ------------------------------------------------
        Paint fill_paint() const;

        void set_fill_paint(const Paint &p_fill_paint);

        Paint stroke_paint() const;

        void set_stroke_paint(const Paint &p_stroke_paint);

        float line_width() const;

        void set_line_width(float p_line_width);

        LineCap line_cap() const;

        void set_line_cap(LineCap p_line_cap);

        LineJoin line_join() const;

        void set_line_join(LineJoin p_line_join);

        float miter_limit() const;

        void set_miter_limit(float p_miter_limit);

        float shadow_blur() const;

        void set_shadow_blur(float p_shadow_blur);

        ColorU shadow_color() const;

        void set_shadow_color(const ColorU &p_shadow_color);

        Vec2<float> shadow_offset() const;

        void set_shadow_offset(float p_shadow_offset_x, float p_shadow_offset_y);

        std::vector<float> line_dash() const;

        void set_line_dash(const std::vector<float> &p_line_dash);

        float line_dash_offset() const;

        void set_line_dash_offset(float p_line_dash_offset);
        // ------------------------------------------------

        // Drawing ops.
        // ------------------------------------------------
        /// Fill a shape.
        void fill_shape(Shape p_shape, FillRule p_fill_rule);

        /// Stroke a shape.
        void stroke_shape(Shape p_shape);
        // ------------------------------------------------

        /// Rebuild and redraw the scene.
        void build_and_render();

        /// Clear the scene.
        void clear();

        std::shared_ptr<Scene> get_scene() const;

        std::shared_ptr<Texture> get_dest_texture();

        /**
         * @brief Load a SVG file into cubic Bezier curves.
         * @param canvas Target canvas.
         * @param input SVG file content.
         */
        void load_svg(const std::string& input);

    private:
        std::shared_ptr<Driver> driver;

        State current_state;
        std::vector<State> saved_states;

        /// Scene
        std::shared_ptr<Scene> scene;

        /// Scene builder and renderer.
#ifndef PATHFINDER_USE_D3D11
        SceneBuilderD3D9 scene_builder;
        std::shared_ptr<RendererD3D9> renderer;
#else
        SceneBuilderD3D11 scene_builder;
        std::shared_ptr<RendererD3D11> renderer;
#endif

        /**
         * Adds a shape.
         * @param p_shape Shape to add.
         * @param p_shape_op Fill/Stroke.
         * @param p_fill_rule Winding/Even-Odd.
         */
        void push_shape(Shape p_shape,
                        ShapeOp p_shape_op,
                        FillRule p_fill_rule);
    };
}
#endif //PATHFINDER_CANVAS_H
