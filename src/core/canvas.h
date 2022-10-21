#ifndef PATHFINDER_CANVAS_H
#define PATHFINDER_CANVAS_H

#include <memory>

#include "../gpu/driver.h"
#include "d3d11/renderer.h"
#include "d3d11/scene_builder.h"
#include "d3d9/renderer.h"
#include "d3d9/scene_builder.h"

namespace Pathfinder {

enum class PathOp {
    Fill,
    Stroke,
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
    Vec2F shadow_offset;

    float global_alpha = 1;
    BlendMode global_composite_operation;

    std::shared_ptr<uint32_t> clip_path; // Optional
};

/// Equivalent to SVG path.
class Path2d {
public:
    // Basic geometries.
    // -----------------------------------------------
    void close_path();

    void move_to(float x, float y);

    void line_to(float x, float y);

    void quadratic_curve_to(float cx, float cy, float x, float y);

    void bezier_curve_to(float cx, float cy, float cx1, float cy1, float x, float y);
    // -----------------------------------------------

    // Advanced geometries.
    // -----------------------------------------------
    void add_line(const Vec2F &start, const Vec2F &end);

    void add_rect(const RectF &rect, float corner_radius = 0);

    void add_circle(const Vec2F &center, float radius);
    // -----------------------------------------------

    Outline into_outline();

private:
    Contour current_contour;

    Outline outline;

    void flush_current_contour();
};

/// Normally, we only need one canvas to render multiple scenes.
class Canvas {
public:
    Canvas(const std::shared_ptr<Driver> &p_driver);

    /// Clears the current canvas.
    void clear();

    void set_empty_dest_texture(uint32_t p_width, uint32_t p_height);

    // Canvas state.
    // ------------------------------------------------
    // Line styles
    float line_width() const;

    void set_line_width(float p_line_width);

    LineCap line_cap() const;

    void set_line_cap(LineCap p_line_cap);

    LineJoin line_join() const;

    void set_line_join(LineJoin p_line_join);

    float miter_limit() const;

    void set_miter_limit(float p_miter_limit);

    std::vector<float> line_dash() const;

    void set_line_dash(const std::vector<float> &p_line_dash);

    float line_dash_offset() const;

    void set_line_dash_offset(float p_line_dash_offset);

    // Fill and stroke styles
    Paint fill_paint() const;

    void set_fill_paint(const Paint &p_fill_paint);

    Paint stroke_paint() const;

    void set_stroke_paint(const Paint &p_stroke_paint);

    // Shadows

    float shadow_blur() const;

    void set_shadow_blur(float p_shadow_blur);

    ColorU shadow_color() const;

    void set_shadow_color(const ColorU &p_shadow_color);

    Vec2F shadow_offset() const;

    void set_shadow_offset(float p_shadow_offset_x, float p_shadow_offset_y);

    Transform2 get_transform() const;

    void set_transform(const Transform2 &p_transform);

    void set_global_alpha(float new_global_alpha);

    void set_global_composite_operation(BlendMode new_composite_operation);
    // ------------------------------------------------

    // Drawing paths.
    // ------------------------------------------------
    void fill_path(Path2d &path2d, FillRule fill_rule);

    void stroke_path(Path2d &path2d);

    void clip_path(Path2d &path2d, FillRule fill_rule);
    // ------------------------------------------------

    // Drawing images
    void draw_image(const Image &image, const RectF &dst_location);

    /// Set the inner scene's view box.
    /// Global control of path clipping.
    void set_size(const Vec2<int> &size);

    Vec2<int> get_size() const;

    /// Local control of path clipping.
    /// Will force bounds of outlines drawn henceforward be within the set clipping box.
    void set_clipping_box(const RectF &box);

    void unset_clipping_box();

    void resize_dest_texture(float p_size_x, float p_size_y);

    /// Returns the inner scene.
    std::shared_ptr<Scene> get_scene() const;

    void set_scene(const std::shared_ptr<Scene> &p_scene);

    /// Returns the inner scene, replacing it with a blank scene.
    std::shared_ptr<Scene> take_scene();

    std::shared_ptr<Scene> replace_scene(const std::shared_ptr<Scene> &new_scene);

    void set_dest_texture(const std::shared_ptr<Texture> &texture);

    std::shared_ptr<Texture> get_dest_texture();

    // Canvas state

    void save_state();

    void restore_state();

    void draw();

private:
    /**
     * Adds an outline.
     * @param outline Outline to add
     * @param path_op Fill/Stroke
     * @param fill_rule Winding/Even-Odd
     */
    void push_path(Outline &outline, PathOp path_op, FillRule fill_rule);

private:
    std::shared_ptr<Scene> scene;

    // State management.
    State current_state;
    std::vector<State> saved_states;

    std::shared_ptr<Texture> dest_texture;

    // TODO: Should be replaced with clip path.
    RectF clipping_box;

    /// Scene renderer.
    std::shared_ptr<Renderer> renderer;

    /// Rendering API related.
    std::shared_ptr<Driver> driver;
};

} // namespace Pathfinder

#endif // PATHFINDER_CANVAS_H
