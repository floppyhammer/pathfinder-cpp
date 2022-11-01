#include "canvas.h"

#include <utility>

#include "../common/global_macros.h"
#include "../common/io.h"
#include "../common/logger.h"
#include "../common/timestamp.h"
#include "dash.h"
#include "stroke.h"

namespace Pathfinder {

struct ShadowBlurRenderTargetInfo {
    /// Render target ids.
    RenderTargetId id_x;
    RenderTargetId id_y;
    /// Shadow color.
    ColorU color;
    /// Shadow bounds.
    RectI bounds;
    /// Blur size.
    float sigma = 0;
};

/**
 * Push shadow blur render targets.
 * @param scene Canvas scene.
 * @param current_state Canvas state.
 * @param outline_bounds Original path bounds.
 */
ShadowBlurRenderTargetInfo push_shadow_blur_render_targets(const std::shared_ptr<Driver> &driver,
                                                           Scene &scene,
                                                           State &current_state,
                                                           RectF outline_bounds) {
    ShadowBlurRenderTargetInfo shadow_blur_info;

    if (current_state.shadow_blur == 0.f) {
        return shadow_blur_info;
    }

    auto sigma = current_state.shadow_blur * 0.5f;

    // Bounds expansion caused by blurring.
    auto bounds = outline_bounds.dilate(sigma * 3.f).round_out().to_i32();

    shadow_blur_info.id_y = scene.push_render_target(RenderTarget(driver, bounds.size()));
    shadow_blur_info.id_x = scene.push_render_target(RenderTarget(driver, bounds.size()));

    shadow_blur_info.sigma = sigma;
    shadow_blur_info.bounds = bounds;

    shadow_blur_info.color = current_state.shadow_color;

    return shadow_blur_info;
}

/**
 * Composite shadow blur render targets.
 * @param scene Canvas scene.
 * @param info Shadow blur render target info.
 */
void composite_shadow_blur_render_targets(Scene &scene, const ShadowBlurRenderTargetInfo &info) {
    if (info.sigma == 0.f) {
        return;
    }

    auto pattern_x = Pattern::from_render_target(info.id_x, info.bounds.size());
    auto pattern_y = Pattern::from_render_target(info.id_y, info.bounds.size());
    pattern_y.apply_transform(Transform2::from_translation(info.bounds.origin().to_f32()));

    auto filter = PatternFilter();

    filter.type = PatternFilter::Type::Blur;
    filter.blur.sigma = info.sigma;

    filter.blur.direction = BlurDirection::X;
    pattern_x.set_filter(filter);

    filter.blur.direction = BlurDirection::Y;
    pattern_y.set_filter(filter);

    auto paint_x = Paint::from_pattern(pattern_x);
    paint_x.set_base_color(info.color);
    auto paint_y = Paint::from_pattern(pattern_y);
    paint_y.set_base_color(info.color);

    auto paint_id_x = scene.push_paint(paint_x);
    auto paint_id_y = scene.push_paint(paint_y);

    // A rect path used to blur the shadow path.
    // Judging by the size, this path will be drawn to a small texture.
    DrawPath path_x;
    {
        Path2d path2d;
        path2d.add_rect({{0, 0}, info.bounds.size().to_f32()});
        path_x.outline = path2d.into_outline();
    }
    path_x.paint = paint_id_x;

    // A rect path used to blur the shadow path.
    // Judging by the size, this path will be drawn to the final texture.
    DrawPath path_y;
    {
        Path2d path2d;
        path2d.add_rect(info.bounds.to_f32());
        path_y.outline = path2d.into_outline();
    }
    path_y.paint = paint_id_y;

    // Pop viewport x.
    scene.pop_render_target();

    // This path goes to the blur viewport y, with the viewport x as the color texture.
    scene.push_draw_path(path_x);

    // Pop viewport y.
    scene.pop_render_target();

    // This path goes to the canvas viewport, with the viewport y as the color texture.
    scene.push_draw_path(path_y);
}

Canvas::Canvas(const std::shared_ptr<Driver> &_driver) : driver(_driver) {
    // Create the renderer.
#ifndef PATHFINDER_USE_D3D11
    renderer = std::make_shared<RendererD3D9>(driver);
#else
    renderer = std::make_shared<RendererD3D11>(driver);
#endif

    // Set up pipelines.
    renderer->set_up_pipelines();

    scene = std::make_shared<Scene>(0, RectF(0, 0, 0, 0));
}

void Canvas::set_empty_dest_texture(uint32_t width, uint32_t height) {
    set_dest_texture(driver->create_texture(width, height, TextureFormat::Rgba8Unorm));
}

void Canvas::push_path(Outline &outline, PathOp path_op, FillRule fill_rule) {
    // Get paint and push it to the scene's palette.
    Paint paint = path_op == PathOp::Fill ? fill_paint() : stroke_paint();
    auto paint_id = scene->push_paint(paint);

    auto transform = current_state.transform;
    auto clip_path = current_state.clip_path;
    auto blend_mode = current_state.global_composite_operation;

    // Apply transform to the outline.
    outline.transform(transform);

    // Add shadow.
    if (current_state.shadow_color.is_opaque()) {
        // Copy outline.
        Outline shadow_outline = outline;

        // Set shadow offset.
        shadow_outline.transform(Transform2::from_translation(current_state.shadow_offset));

        auto shadow_blur_info = push_shadow_blur_render_targets(driver, *scene, current_state, shadow_outline.bounds);

        shadow_outline.transform(Transform2::from_translation(-shadow_blur_info.bounds.origin().to_f32()));

        // Per spec the shadow must respect the alpha of the shadowed path, but otherwise have
        // the color of the shadow paint.
        auto shadow_paint = paint;
        auto shadow_base_alpha = shadow_paint.get_base_color().a;
        auto shadow_color = current_state.shadow_color.to_f32();
        shadow_color.a = shadow_color.a * (float)shadow_base_alpha / 255.f;
        shadow_paint.set_base_color(ColorU(shadow_color));

        auto overlay = shadow_paint.get_overlay();
        if (overlay) {
            overlay->composite_op = PaintCompositeOp::DestIn;
        }

        auto shadow_paint_id = scene->push_paint(shadow_paint);

        // Create a new draw path from the outline.
        DrawPath path;
        path.outline = shadow_outline;
        path.paint = shadow_paint_id;
        path.fill_rule = fill_rule;
        path.blend_mode = blend_mode;

        // This path goes to the blur viewport x.
        scene->push_draw_path(path);

        composite_shadow_blur_render_targets(*scene, shadow_blur_info);
    }

    DrawPath path;
    path.outline = outline;
    path.paint = paint_id;
    path.clip_path = clip_path;
    path.fill_rule = fill_rule;
    path.blend_mode = blend_mode;

    scene->push_draw_path(path);
}

void Canvas::fill_path(Path2d &path2d, FillRule fill_rule) {
    if (current_state.fill_paint.is_opaque()) {
        auto outline = path2d.into_outline();
        push_path(outline, PathOp::Fill, fill_rule);
    }
}

void Canvas::stroke_path(Path2d &path2d) {
    // Set stroke style.
    auto style = StrokeStyle();
    style.line_width = line_width();
    style.line_join = line_join();
    style.miter_limit = miter_limit();
    style.line_cap = line_cap();

    // No need to draw an invisible stroke.
    if (current_state.stroke_paint.is_opaque() && style.line_width > 0) {
        auto outline = path2d.into_outline();

        // Do dash before converting stroke to fill.
        if (!current_state.line_dash.empty()) {
            auto dasher = OutlineDash(outline, current_state.line_dash, 0);
            dasher.dash();
            outline = dasher.into_outline();
        }

        auto stroke_to_fill = OutlineStrokeToFill(outline, style);

        // Do stroking.
        stroke_to_fill.offset();

        auto stroke_outline = stroke_to_fill.into_outline();

        // Even-Odd fill rule is not applicable for strokes.
        push_path(stroke_outline, PathOp::Stroke, FillRule::Winding);
    }
}

void Canvas::clip_path(Path2d &path, FillRule fill_rule) {
    auto outline = path.into_outline();
    outline.transform(current_state.transform);

    ClipPath clip_path;
    clip_path.outline = outline;
    clip_path.fill_rule = fill_rule;
    clip_path.clip_path = current_state.clip_path;

    uint32_t clip_path_id = scene->push_clip_path(clip_path);
    current_state.clip_path = std::make_shared<uint32_t>(clip_path_id);
}

Paint Canvas::fill_paint() const {
    return current_state.fill_paint;
}

void Canvas::set_fill_paint(const Paint &p_fill_paint) {
    current_state.fill_paint = p_fill_paint;
}

Paint Canvas::stroke_paint() const {
    return current_state.stroke_paint;
}

void Canvas::set_stroke_paint(const Paint &p_stroke_paint) {
    current_state.stroke_paint = p_stroke_paint;
}

float Canvas::line_width() const {
    return current_state.line_width;
}

void Canvas::set_line_width(float new_line_width) {
    current_state.line_width = new_line_width;
}

LineCap Canvas::line_cap() const {
    return current_state.line_cap;
}

void Canvas::set_line_cap(LineCap new_line_cap) {
    current_state.line_cap = new_line_cap;
}

LineJoin Canvas::line_join() const {
    return current_state.line_join;
}

void Canvas::set_line_join(LineJoin new_line_join) {
    current_state.line_join = new_line_join;
}

float Canvas::miter_limit() const {
    return current_state.miter_limit;
}

void Canvas::set_miter_limit(float new_miter_limit) {
    current_state.miter_limit = new_miter_limit;
}

float Canvas::shadow_blur() const {
    return current_state.shadow_blur;
}

void Canvas::set_shadow_blur(float new_shadow_blur) {
    current_state.shadow_blur = new_shadow_blur;
}

ColorU Canvas::shadow_color() const {
    return current_state.shadow_color;
}

void Canvas::set_shadow_color(const ColorU &new_shadow_color) {
    current_state.shadow_color = new_shadow_color;
}

Vec2F Canvas::shadow_offset() const {
    return current_state.shadow_offset;
}

void Canvas::set_shadow_offset(const Vec2F &new_shadow_offset) {
    current_state.shadow_offset = new_shadow_offset;
}

std::vector<float> Canvas::line_dash() const {
    return current_state.line_dash;
}

void Canvas::set_line_dash(const std::vector<float> &new_line_dash) {
    current_state.line_dash = new_line_dash;
}

float Canvas::line_dash_offset() const {
    return current_state.line_dash_offset;
}

void Canvas::set_line_dash_offset(float new_line_dash_offset) {
    current_state.line_dash_offset = new_line_dash_offset;
}

Transform2 Canvas::get_transform() const {
    return current_state.transform;
}

void Canvas::set_transform(const Transform2 &new_transform) {
    current_state.transform = new_transform;
}

void Canvas::set_global_alpha(float new_global_alpha) {
    current_state.global_alpha = new_global_alpha;
}

void Canvas::set_global_composite_operation(BlendMode new_composite_operation) {
    current_state.global_composite_operation = new_composite_operation;
}

void Canvas::fill_rect(const RectF &rect) {
    Path2d path;
    path.add_rect(rect);
    fill_path(path, FillRule::Winding);
}

void Canvas::stroke_rect(const RectF &rect) {
    Path2d path;
    path.add_rect(rect);
    stroke_path(path);
}

void Canvas::clear_rect(const RectF &rect) {
    Path2d path;
    path.add_rect(rect);

    auto paint = Paint::from_color(ColorU::transparent_black());
    auto paint_id = scene->push_paint(paint);

    auto outline = path.into_outline();
    outline.transform(current_state.transform);

    DrawPath draw_path;
    draw_path.outline = outline;
    draw_path.paint = paint_id;
    draw_path.blend_mode = BlendMode::Clear;
    scene->push_draw_path(draw_path);
}

void Canvas::draw_image(const Image &image, const RectF &dst_rect) {
    auto pattern = Pattern::from_image(image);

    // Set the whole image as the src rect.
    auto src_rect = RectF({}, pattern.get_size().to_f32());

    draw_subimage(image, src_rect, dst_rect);
}

void Canvas::draw_subimage(const Image &image, const RectF &src_rect, const RectF &dst_rect) {
    auto dst_size = dst_rect.size();
    auto scale = dst_size / src_rect.size();
    auto offset = dst_rect.origin() - src_rect.origin() * scale;
    auto transform = Transform2::from_scale(scale).translate(offset);

    auto pattern = Pattern::from_image(image);
    pattern.apply_transform(transform);

    // Save the current fill paint.
    auto old_fill_paint = current_state.fill_paint;

    current_state.fill_paint = Paint::from_pattern(pattern);
    fill_rect(RectF(dst_rect.origin(), dst_rect.origin() + dst_size));

    // Restore the previous fill paint.
    current_state.fill_paint = old_fill_paint;
}

void Canvas::clear() {
    take_scene();
}

void Canvas::resize_dest_texture(const Vec2I &new_size) {
    if (new_size.x <= 0 || new_size.y <= 0) {
        Logger::error("Invalid dest texture size!", "Canvas");
        return;
    }

    if (dest_texture->get_size() == new_size) {
        return;
    }

    set_dest_texture(driver->create_texture(new_size.x, new_size.y, TextureFormat::Bgra8Unorm));
}

std::shared_ptr<Scene> Canvas::get_scene() const {
    return scene;
}

void Canvas::set_dest_texture(const std::shared_ptr<Texture> &texture) {
    dest_texture = texture;
    renderer->set_dest_texture(texture);
}

std::shared_ptr<Texture> Canvas::get_dest_texture() {
    return dest_texture;
}

void Canvas::save_state() {
    saved_states.push_back(current_state);
}

void Canvas::restore_state() {
    if (!saved_states.empty()) {
        current_state = saved_states.back();
        saved_states.pop_back();
    }
}

void Canvas::draw() {
    scene->build_and_render(renderer);
}

void Canvas::set_scene(const std::shared_ptr<Scene> &p_scene) {
    scene = p_scene;
}

std::shared_ptr<Scene> Canvas::take_scene() {
    auto taken_scene = scene;

    scene = std::make_shared<Scene>(0, scene->get_view_box());

    return taken_scene;
}

std::shared_ptr<Scene> Canvas::replace_scene(const std::shared_ptr<Scene> &new_scene) {
    auto taken_scene = scene;

    scene = new_scene;

    return taken_scene;
}

void Canvas::set_size(const Vec2I &new_size) {
    auto new_view_box = RectI({}, new_size).to_f32();
    scene->set_bounds(new_view_box);
    scene->set_view_box(new_view_box);
}

Vec2I Canvas::get_size() const {
    return scene->get_view_box().size().ceil();
}

// Path2d
// -----------------------------

void Path2d::close_path() {
    current_contour.close();
}

void Path2d::move_to(float x, float y) {
    flush_current_contour();
    current_contour.push_endpoint({x, y});
}

void Path2d::line_to(float x, float y) {
    current_contour.push_endpoint({x, y});
}

void Path2d::quadratic_to(float cx, float cy, float x, float y) {
    current_contour.push_quadratic({cx, cy}, {x, y});
}

void Path2d::cubic_to(float cx0, float cy0, float cx1, float cy1, float x, float y) {
    current_contour.push_cubic({cx0, cy0}, {cx1, cy1}, {x, y});
}

void Path2d::add_line(const Vec2F &start, const Vec2F &end) {
    if (start == end) return;

    move_to(start.x, start.y);
    line_to(end.x, end.y);
}

const float CIRCLE_RATIO = 0.552284749831; // 4.0f * (sqrt(2.0f) - 1.0f) / 3.0f

void Path2d::add_rect(const RectF &rect, float corner_radius) {
    if (rect.size().x == 0 || rect.size().y == 0) return;

    if (corner_radius <= 0) {
        move_to(rect.min_x(), rect.min_y());
        line_to(rect.max_x(), rect.min_y());
        line_to(rect.max_x(), rect.max_y());
        line_to(rect.min_x(), rect.max_y());
        close_path();

        return;
    }

    // Corner radius can't be greater than the half of the shorter line of the rect.
    corner_radius = std::min(corner_radius, std::min(rect.width(), rect.height()) * 0.5f);

    // See https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves.
    float adjusted_radius = corner_radius * CIRCLE_RATIO;

    move_to(rect.min_x(), rect.min_y() + corner_radius);
    cubic_to(rect.min_x(),
             rect.min_y() + corner_radius - adjusted_radius,
             rect.min_x() + corner_radius - adjusted_radius,
             rect.min_y(),
             rect.min_x() + corner_radius,
             rect.min_y());
    line_to(rect.max_x() - corner_radius, rect.min_y());
    cubic_to(rect.max_x() - corner_radius + adjusted_radius,
             rect.min_y(),
             rect.max_x(),
             rect.min_y() + corner_radius - adjusted_radius,
             rect.max_x(),
             rect.min_y() + corner_radius);
    line_to(rect.max_x(), rect.max_y() - corner_radius);
    cubic_to(rect.max_x(),
             rect.max_y() - corner_radius + adjusted_radius,
             rect.max_x() - corner_radius + adjusted_radius,
             rect.max_y(),
             rect.max_x() - corner_radius,
             rect.max_y());
    line_to(rect.min_x() + corner_radius, rect.max_y());
    cubic_to(rect.min_x() + corner_radius - adjusted_radius,
             rect.max_y(),
             rect.min_x(),
             rect.max_y() - corner_radius + adjusted_radius,
             rect.min_x(),
             rect.max_y() - corner_radius);
    close_path();
}

void Path2d::add_circle(const Vec2F &center, float radius) {
    if (radius == 0) return;

    // See https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves.
    float adjusted_radius = radius * CIRCLE_RATIO;

    move_to(center.x, center.y - radius);
    cubic_to(center.x + adjusted_radius,
             center.y - radius,
             center.x + radius,
             center.y - adjusted_radius,
             center.x + radius,
             center.y);
    cubic_to(center.x + radius,
             center.y + adjusted_radius,
             center.x + adjusted_radius,
             center.y + radius,
             center.x,
             center.y + radius);
    cubic_to(center.x - adjusted_radius,
             center.y + radius,
             center.x - radius,
             center.y + adjusted_radius,
             center.x - radius,
             center.y);
    cubic_to(center.x - radius,
             center.y - adjusted_radius,
             center.x - adjusted_radius,
             center.y - radius,
             center.x,
             center.y - radius);
    close_path();
}

Outline Path2d::into_outline() {
    flush_current_contour();
    return outline;
}

void Path2d::flush_current_contour() {
    if (!current_contour.is_empty()) {
        outline.push_contour(current_contour);
        current_contour = Contour();
    }
}

// -----------------------------

} // namespace Pathfinder
