#ifndef PATHFINDER_CANVAS_H
#define PATHFINDER_CANVAS_H

#include <memory>

#include "../gpu/driver.h"
#include "d3d11/renderer.h"
#include "d3d11/scene_builder.h"
#include "d3d9/renderer.h"
#include "d3d9/scene_builder.h"

namespace Pathfinder {
enum PathOp {
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

/// Canvas state.
struct State {
    Transform2 transform;

    float line_width = 0;
    LineCap line_cap = LineCap::Butt;
    LineJoin line_join = LineJoin::Miter;
    float miter_limit = 10;

    // Dash.
    std::vector<float> line_dash;
    float line_dash_offset = 0;

    Paint fill_paint;
    Paint stroke_paint;

    ColorU shadow_color;
    float shadow_blur = 0;
    Vec2<float> shadow_offset;

    BlendMode global_composite_operation;
};

/// Normally, we only need one canvas to render multiple scenes.
class Canvas {
public:
    Canvas(const std::shared_ptr<Driver> &p_driver);

    void set_empty_scene(const Rect<float> &view_box);

    void set_empty_dest_texture(float p_size_x, float p_size_y);

    // Change state.
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

    void set_transform(const Transform2 &p_transform);
    // ------------------------------------------------

    // Drawing ops.
    // ------------------------------------------------
    /// Fill a shape.
    void fill_path(Outline p_outline, FillRule p_fill_rule);

    /// Stroke a path.
    void stroke_path(Outline p_outline);
    // ------------------------------------------------

    /// Build the scene by SceneBuilder.
    void build();

    /// Render the scene by Renderer.
    void render();

    /// A convenience method to build and render a scene.
    void build_and_render();

    /// Clear the scene.
    void clear();

    void resize(float p_size_x, float p_size_y);

    void set_view_box(const Rect<float> &view_box);

    std::shared_ptr<SceneBuilder> get_scene_builder() const;

    void set_scene_builder(const std::shared_ptr<SceneBuilder> &p_scene_builder);

    void set_dest_texture(const std::shared_ptr<Texture> &texture);

    std::shared_ptr<Texture> get_dest_texture();

    /**
     * @brief Load a SVG file into the scene.
     * @note We need a copy of the input vector as its content will be modified.
     * @param input SVG file content.
     */
    void load_svg(std::vector<char> input);

    // TODO
    void draw_image();

    // Canvas state

    void save_state();

    void restore_state();

private:
    /**
     * Adds a path.
     * @param p_outline Path to add.
     * @param p_path_op Fill/Stroke.
     * @param p_fill_rule Winding/Even-Odd.
     */
    void push_path(Outline &p_outline, PathOp p_path_op, FillRule p_fill_rule);

private:
    std::shared_ptr<Driver> driver;

    State current_state;
    std::vector<State> saved_states;

    std::shared_ptr<Texture> dest_texture;

    /// Scene builder.
    std::shared_ptr<SceneBuilder> scene_builder;

    /// Scene renderer.
    std::shared_ptr<Renderer> renderer;
};
} // namespace Pathfinder
#endif // PATHFINDER_CANVAS_H
