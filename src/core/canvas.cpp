#include "canvas.h"

#include "../common/global_macros.h"
#include "../common/io.h"
#include "../common/logger.h"
#include "../common/timestamp.h"
#include "dash.h"
#include "stroke.h"

#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>

namespace Pathfinder {
struct ShadowBlurRenderTargetInfo {
    /// Render targets.
    RenderTarget id_x;
    RenderTarget id_y;

    /// Shadow color.
    ColorU color;

    /// Shadow bounds.
    Rect<int> bounds;

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
                                                           Rect<float> outline_bounds) {
    ShadowBlurRenderTargetInfo shadow_blur_info;

    if (current_state.shadow_blur == 0.f) {
        return shadow_blur_info;
    }

    auto sigma = current_state.shadow_blur * 0.5f;

    // Bounds expansion caused by blurring.
    auto bounds = outline_bounds.dilate(sigma * 3.f).round_out().to_i32();

    shadow_blur_info.id_y = scene.push_render_target(driver, bounds.size());
    shadow_blur_info.id_x = scene.push_render_target(driver, bounds.size());

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

    auto pattern_x = Pattern::from_render_target(info.id_x);
    auto pattern_y = Pattern::from_render_target(info.id_y);
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

Canvas::Canvas(const std::shared_ptr<Driver> &p_driver) {
    driver = p_driver;

    // Set up a renderer.
#ifndef PATHFINDER_USE_D3D11
    renderer = std::make_shared<RendererD3D9>(p_driver);
#else
    renderer = std::make_shared<RendererD3D11>(p_driver);
#endif

    renderer->set_up();

    renderer->set_up_pipelines();
}

void Canvas::set_empty_scene(const Rect<float> &view_box) {
    scene = std::make_shared<Scene>(0, view_box);
}

void Canvas::set_empty_dest_texture(float size_x, float size_y) {
    set_dest_texture(driver->create_texture(size_x, size_y, TextureFormat::RGBA8_UNORM));
}

void Canvas::push_path(Outline &p_outline, PathOp p_path_op, FillRule p_fill_rule) {
    // Get paint.
    Paint paint = p_path_op == PathOp::Fill ? fill_paint() : stroke_paint();

    // Push to the scene's palette.
    auto paint_id = scene->push_paint(paint);

    auto transform = current_state.transform;
    auto blend_mode = current_state.global_composite_operation;

    // Apply transform.
    p_outline.transform(transform);

    // Add shadow.
    if (current_state.shadow_color.is_opaque()) {
        // Copy outline.
        Outline shadow_outline = p_outline;

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
        path.fill_rule = p_fill_rule;
        path.blend_mode = blend_mode;

        // This path goes to the blur viewport x.
        scene->push_draw_path(path);

        composite_shadow_blur_render_targets(*scene, shadow_blur_info);
    }

    DrawPath path;
    path.outline = p_outline;
    path.paint = paint_id;
    path.fill_rule = p_fill_rule;
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

void Canvas::set_line_width(float p_line_width) {
    current_state.line_width = p_line_width;
}

LineCap Canvas::line_cap() const {
    return current_state.line_cap;
}

void Canvas::set_line_cap(LineCap p_line_cap) {
    current_state.line_cap = p_line_cap;
}

LineJoin Canvas::line_join() const {
    return current_state.line_join;
}

void Canvas::set_line_join(LineJoin p_line_join) {
    current_state.line_join = p_line_join;
}

float Canvas::miter_limit() const {
    return current_state.miter_limit;
}

void Canvas::set_miter_limit(float p_miter_limit) {
    current_state.miter_limit = p_miter_limit;
}

float Canvas::shadow_blur() const {
    return current_state.shadow_blur;
}

void Canvas::set_shadow_blur(float p_shadow_blur) {
    current_state.shadow_blur = p_shadow_blur;
}

ColorU Canvas::shadow_color() const {
    return current_state.shadow_color;
}

void Canvas::set_shadow_color(const ColorU &p_shadow_color) {
    current_state.shadow_color = p_shadow_color;
}

Vec2<float> Canvas::shadow_offset() const {
    return current_state.shadow_offset;
}

void Canvas::set_shadow_offset(float p_shadow_offset_x, float p_shadow_offset_y) {
    current_state.shadow_offset = {p_shadow_offset_x, p_shadow_offset_y};
}

std::vector<float> Canvas::line_dash() const {
    return current_state.line_dash;
}

void Canvas::set_line_dash(const std::vector<float> &p_line_dash) {
    current_state.line_dash = p_line_dash;
}

float Canvas::line_dash_offset() const {
    return current_state.line_dash_offset;
}

void Canvas::set_line_dash_offset(float p_line_dash_offset) {
    current_state.line_dash_offset = p_line_dash_offset;
}

void Canvas::set_transform(const Transform2 &p_transform) {
    current_state.transform = p_transform;
}

void Canvas::clear() {
    // Create a new scene but keep the ID and the view box.
    scene = std::make_shared<Scene>(scene->id, scene->get_view_box());
}

void Canvas::resize(float p_size_x, float p_size_y) {
    if (p_size_x <= 0 || p_size_y <= 0) {
        return;
    }

    if (dest_texture->get_width() == p_size_x && dest_texture->get_height() == p_size_y) {
        return;
    }

    set_dest_texture(driver->create_texture(p_size_x, p_size_y, TextureFormat::BGRA8_UNORM));
}

void Canvas::set_view_box(const Rect<float> &view_box) {
    scene->set_view_box(view_box);
}

std::shared_ptr<Scene> Canvas::get_scene() const {
    return scene;
}

void Canvas::set_scene(const std::shared_ptr<Scene> &p_scene) {
    scene = p_scene;
}

void Canvas::set_dest_texture(const std::shared_ptr<Texture> &texture) {
    dest_texture = texture;
    renderer->set_dest_texture(texture);
}

std::shared_ptr<Texture> Canvas::get_dest_texture() {
    return dest_texture;
}

/**
 * @brief Convert NanoSVG fill rule enum to our own type.
 * @param line_cap NanoSVG fill rule enum.
 * @return FillRule enum.
 */
FillRule convert_nsvg_fill_rule(char fill_rule) {
    return fill_rule == NSVGfillRule::NSVG_FILLRULE_EVENODD ? FillRule::EvenOdd : FillRule::Winding;
}

/**
 * @brief Convert NanoSVG line cap enum to our own type.
 * @param line_cap NanoSVG line cap enum.
 * @return LineCap enum.
 */
LineCap convert_nsvg_line_cap(char line_cap) {
    switch (line_cap) {
        case NSVGlineCap::NSVG_CAP_ROUND: {
            return LineCap::Round;
        }
        case NSVGlineCap::NSVG_CAP_SQUARE: {
            return LineCap::Square;
        }
        default: {
            return LineCap::Butt;
        }
    }
}

/**
 * @brief Convert NanoSVG line join enum to our own type.
 * @param line_cap NanoSVG line join enum.
 * @return LineJoin enum.
 */
LineJoin convert_nsvg_line_join(char line_join) {
    switch (line_join) {
        case NSVGlineJoin::NSVG_JOIN_ROUND: {
            return LineJoin::Round;
        }
        case NSVGlineJoin::NSVG_JOIN_BEVEL: {
            return LineJoin::Bevel;
        }
        default: {
            return LineJoin::Miter;
        }
    }
}

Paint convert_nsvg_paint(NSVGpaint nsvg_paint) {
    Paint paint;

    // FIXME: Non-Identity transform will cause incorrect gradient (both linear and radical) rendering.
    switch (nsvg_paint.type) {
        case NSVG_PAINT_NONE:
            break;
        case NSVG_PAINT_COLOR: {
            paint = Paint::from_color(ColorU(nsvg_paint.color));

            // Image pattern test.
            if (false) {
                auto image_data = ImageData::from_file("../assets/test.png", false);

                Image image;
                image.pixels = image_data->to_rgba_pixels();
                image.size = {image_data->width, image_data->height};

                auto pattern = Pattern::from_image(image);

                // FIXME: Should assign path transform to the pattern, but NanoSVG cannot provide it.
                //                pattern.transform = Transform2();

                paint = Paint::from_pattern(pattern);
            }
        } break;
        case NSVG_PAINT_LINEAR_GRADIENT:
        case NSVG_PAINT_RADIAL_GRADIENT: {
            auto nsvg_gradient = nsvg_paint.gradient;

            auto gradient_xform = Transform2(nsvg_gradient->xform2);

            auto path_xform = Transform2(nsvg_gradient->xform3);

            Gradient gradient;
            if (nsvg_paint.type == NSVG_PAINT_LINEAR_GRADIENT) {
                Vec2F from = {nsvg_gradient->x1, nsvg_gradient->y1};
                Vec2F to = {nsvg_gradient->x2, nsvg_gradient->y2};

                // Apply gradient transform.
                from = path_xform * gradient_xform * from;
                to = path_xform * gradient_xform * to;

                gradient = Gradient::linear(LineSegmentF(from, to));
            } else {
                Vec2F from = {nsvg_gradient->cx, nsvg_gradient->cy};
                Vec2F to = {nsvg_gradient->fx, nsvg_gradient->fy};

                gradient = Gradient::radial(LineSegmentF(from, to), Vec2F(0.0, nsvg_gradient->r));

                gradient.geometry.radial.transform = path_xform * gradient_xform;
            }

            // TODO: Allow change color texture sampling mode.
            switch (nsvg_gradient->spread) {
                case NSVG_SPREAD_PAD: {
                    gradient.wrap = GradientWrap::Clamp;
                } break;
                case NSVG_SPREAD_REFLECT: {
                    Logger::error("Reflect gradient spread is not supported!");
                } break;
                case NSVG_SPREAD_REPEAT: {
                    gradient.wrap = GradientWrap::Repeat;
                } break;
            };

            // Get stops.
            for (int i = 0; i < nsvg_gradient->nstops; i++) {
                gradient.add_color_stop(ColorU(nsvg_gradient->stops[i].color), nsvg_gradient->stops[i].offset);
            }

            paint = Paint::from_gradient(gradient);
        } break;
    }

    return paint;
}

void Canvas::load_svg(std::vector<char> input) {
    if (input.empty()) {
        Logger::error("SVG input is empty!", "Canvas");
        return;
    }

    // Load the SVG image.
    NSVGimage *image = nsvgParse(input.data(), "px", 96);

    // Check if image loading is successful.
    if (image == nullptr) {
        Logger::error("NanoSVG loading image failed!", "Canvas");
        return;
    }

    // Extract paths, contours and points from the SVG image.
    // Notable: NSVGshape equals to our Path, and NSVGpath equals to our Contour (Sub-Path).
    for (NSVGshape *nsvg_shape = image->shapes; nsvg_shape != nullptr; nsvg_shape = nsvg_shape->next) {
        Path2d path2d;

        for (NSVGpath *nsvg_path = nsvg_shape->paths; nsvg_path != nullptr; nsvg_path = nsvg_path->next) {
            path2d.move_to(nsvg_path->pts[0], nsvg_path->pts[1]);

            // -6 or -3, both will do, probably.
            for (int point_index = 0; point_index < nsvg_path->npts - 3; point_index += 3) {
                // * 2 because a point has x and y components.
                float *p = &nsvg_path->pts[point_index * 2];
                path2d.bezier_curve_to(p[2], p[3], p[4], p[5], p[6], p[7]);
            }

            if (nsvg_path->closed) {
                path2d.close_path();
            }
        }

        // Shadow test.
        if (false) {
            set_shadow_color(ColorU::green());
            set_shadow_blur(8);
        }

        // Set dash.
        set_line_dash_offset(nsvg_shape->strokeDashOffset);
        set_line_dash(
            std::vector<float>(nsvg_shape->strokeDashArray, nsvg_shape->strokeDashArray + nsvg_shape->strokeDashCount));

        // Add fill.
        set_fill_paint(convert_nsvg_paint(nsvg_shape->fill));
        fill_path(path2d, convert_nsvg_fill_rule(nsvg_shape->fillRule));

        // Add stroke.
        set_line_join(convert_nsvg_line_join(nsvg_shape->strokeLineJoin));
        set_miter_limit(nsvg_shape->miterLimit);
        set_line_cap(convert_nsvg_line_cap(nsvg_shape->strokeLineCap));
        set_line_width(nsvg_shape->strokeWidth);
        set_stroke_paint(convert_nsvg_paint(nsvg_shape->stroke));
        stroke_path(path2d);
    }

    // Clean up NanoSVG image.
    nsvgDelete(image);
}

void Canvas::draw_image() {}

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

void Path2d::quadratic_curve_to(float cx, float cy, float x, float y) {
    current_contour.push_quadratic({cx, cy}, {x, y});
}

void Path2d::bezier_curve_to(float cx0, float cy0, float cx1, float cy1, float x, float y) {
    current_contour.push_cubic({cx0, cy0}, {cx1, cy1}, {x, y});
}

void Path2d::add_line(const Vec2<float> &start, const Vec2<float> &end) {
    if (start == end) return;

    move_to(start.x, start.y);
    line_to(end.x, end.y);
}

const float CIRCLE_RATIO = 0.552284749831; // 4.0f * (sqrt(2.0f) - 1.0f) / 3.0f

void Path2d::add_rect(const Rect<float> &rect, float corner_radius) {
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
    bezier_curve_to(rect.min_x(),
                    rect.min_y() + corner_radius - adjusted_radius,
                    rect.min_x() + corner_radius - adjusted_radius,
                    rect.min_y(),
                    rect.min_x() + corner_radius,
                    rect.min_y());
    line_to(rect.max_x() - corner_radius, rect.min_y());
    bezier_curve_to(rect.max_x() - corner_radius + adjusted_radius,
                    rect.min_y(),
                    rect.max_x(),
                    rect.min_y() + corner_radius - adjusted_radius,
                    rect.max_x(),
                    rect.min_y() + corner_radius);
    line_to(rect.max_x(), rect.max_y() - corner_radius);
    bezier_curve_to(rect.max_x(),
                    rect.max_y() - corner_radius + adjusted_radius,
                    rect.max_x() - corner_radius + adjusted_radius,
                    rect.max_y(),
                    rect.max_x() - corner_radius,
                    rect.max_y());
    line_to(rect.min_x() + corner_radius, rect.max_y());
    bezier_curve_to(rect.min_x() + corner_radius - adjusted_radius,
                    rect.max_y(),
                    rect.min_x(),
                    rect.max_y() - corner_radius + adjusted_radius,
                    rect.min_x(),
                    rect.max_y() - corner_radius);
    close_path();
}

void Path2d::add_circle(const Vec2<float> &center, float radius) {
    if (radius == 0) return;

    // See https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves.
    float adjusted_radius = radius * CIRCLE_RATIO;

    move_to(center.x, center.y - radius);
    bezier_curve_to(center.x + adjusted_radius,
                    center.y - radius,
                    center.x + radius,
                    center.y - adjusted_radius,
                    center.x + radius,
                    center.y);
    bezier_curve_to(center.x + radius,
                    center.y + adjusted_radius,
                    center.x + adjusted_radius,
                    center.y + radius,
                    center.x,
                    center.y + radius);
    bezier_curve_to(center.x - adjusted_radius,
                    center.y + radius,
                    center.x - radius,
                    center.y + adjusted_radius,
                    center.x - radius,
                    center.y);
    bezier_curve_to(center.x - radius,
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
