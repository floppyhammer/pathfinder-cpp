#ifndef PATHFINDER_CANVAS_H
#define PATHFINDER_CANVAS_H

#include <memory>

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
    BlendMode global_composite_operation = BlendMode::SrcOver;

    // The clip path is scene-dependent, so remember to clear it when switching scene.
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

    void quadratic_to(float cx, float cy, float x, float y);

    void cubic_to(float cx, float cy, float cx1, float cy1, float x, float y);
    // -----------------------------------------------

    // Advanced geometries.
    // -----------------------------------------------
    void add_line(const Vec2F &start, const Vec2F &end);

    void add_rect(const RectF &rect, float corner_radius = 0);

    void add_circle(const Vec2F &center, float radius);
    // -----------------------------------------------

    /// Returns the outline.
    Outline into_outline();

private:
    Contour current_contour;

    Outline outline;

    void flush_current_contour();
};

/// Normally, we only need one canvas to render multiple scenes.
class Canvas {
public:
    explicit Canvas(const std::shared_ptr<Driver> &_driver);

    /// Clears the current canvas.
    void clear();

    void set_dst_texture(const std::shared_ptr<Texture> &new_dst_texture);

    std::shared_ptr<Texture> get_dst_texture();

    // Canvas state.
    // ------------------------------------------------
    // Line styles
    float line_width() const;

    void set_line_width(float new_line_width);

    LineCap line_cap() const;

    void set_line_cap(LineCap new_line_cap);

    LineJoin line_join() const;

    void set_line_join(LineJoin new_line_join);

    float miter_limit() const;

    void set_miter_limit(float new_miter_limit);

    std::vector<float> line_dash() const;

    void set_line_dash(const std::vector<float> &new_line_dash);

    float line_dash_offset() const;

    void set_line_dash_offset(float new_line_dash_offset);

    // Fill and stroke styles
    Paint fill_paint() const;

    void set_fill_paint(const Paint &new_fill_paint);

    Paint stroke_paint() const;

    void set_stroke_paint(const Paint &new_stroke_paint);

    // Shadows

    float shadow_blur() const;

    void set_shadow_blur(float new_shadow_blur);

    ColorU shadow_color() const;

    void set_shadow_color(const ColorU &new_shadow_color);

    Vec2F shadow_offset() const;

    void set_shadow_offset(const Vec2F &new_shadow_offset);

    Transform2 get_transform() const;

    void set_transform(const Transform2 &new_transform);

    void set_global_alpha(float new_global_alpha);

    void set_global_composite_operation(BlendMode new_composite_operation);
    // ------------------------------------------------

    // Drawing paths.
    // ------------------------------------------------
    void fill_path(Path2d &path2d, FillRule fill_rule);

    void stroke_path(Path2d &path2d);

    void clip_path(Path2d &path2d, FillRule fill_rule);
    // ------------------------------------------------

    // Drawing rectangles

    void fill_rect(const RectF &rect);

    void stroke_rect(const RectF &rect);

    void clear_rect(const RectF &rect);

    // Drawing images

    void draw_image(const Image &image, const RectF &dst_rect);

    void draw_subimage(const Image &image, const RectF &src_rect, const RectF &dst_rect);

    void draw_render_target(const RenderTargetId &render_target_id, const RectF &dst_rect);

    void draw_sub_render_target(const RenderTargetId &render_target_id, const RectF &src_rect, const RectF &dst_rect);

    /// Set the inner scene's view box.
    /// Global control of path clipping.
    void set_size(const Vec2I &size);

    Vec2I get_size() const;

    /// Returns the inner scene.
    std::shared_ptr<Scene> get_scene() const;

    void set_scene(const std::shared_ptr<Scene> &new_scene);

    std::shared_ptr<Driver> get_driver() const;

    /// Returns the inner scene, replacing it with a blank scene.
    std::shared_ptr<Scene> take_scene();

    // Canvas state

    void save_state();

    void restore_state();

    void draw();

    // Extensions

    Pattern create_pattern_from_canvas(Canvas &canvas, const Transform2 &transform);

private:
    /**
     * Adds an outline.
     * @param outline Outline to add
     * @param path_op Fill/Stroke
     * @param fill_rule Winding/Even-Odd
     */
    void push_path(Outline &outline, PathOp path_op, FillRule fill_rule);

private:
    // State management.
    State current_state;
    std::vector<State> saved_states;

    std::shared_ptr<Scene> scene;

    /// Rendering API related.
    std::shared_ptr<Driver> driver;

    /// Scene renderer.
    std::shared_ptr<Renderer> renderer;
};

} // namespace Pathfinder

#endif // PATHFINDER_CANVAS_H
