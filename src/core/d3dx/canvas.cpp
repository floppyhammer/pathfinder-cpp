#include "canvas.h"

#include "stroke.h"
#include "dash.h"
#include "../../common/timestamp.h"
#include "../../common/global_macros.h"
#include "../../common/logger.h"

#define NANOSVG_IMPLEMENTATION

#include <nanosvg.h>

#include <utility>

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
     * @param outline_bounds Original shape bounds.
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

        auto sigma = info.sigma;

        PatternFilter pf;
        pf.type = PatternFilter::Type::Blur;
        pf.blur.sigma = sigma;

        pf.blur.direction = BlurDirection::X;
        pattern_x.set_filter(pf);
        pf.blur.direction = BlurDirection::Y;
        pattern_y.set_filter(pf);

        auto paint_x = Paint::from_pattern(pattern_x);
        paint_x.set_base_color(info.color);
        auto paint_y = Paint::from_pattern(pattern_y);
        paint_y.set_base_color(info.color);
        auto paint_id_x = scene.push_paint(paint_x);
        auto paint_id_y = scene.push_paint(paint_y);

        // A rect shape used to blur the shadow shape.
        // Judging by the size, this shape will be drawn to a small texture.
        Shape path_x;
        path_x.add_rect(Rect<float>(Vec2<float>(0.0), info.bounds.size().to_f32()));
        path_x.paint = paint_id_x;

        // A rect shape used to blur the shadow shape.
        // Judging by the size, this shape will be drawn to the final texture.
        Shape path_y;
        path_y.add_rect(info.bounds.to_f32());
        path_y.paint = paint_id_y;

        // Pop viewport x.
        scene.pop_render_target();

        // This shape goes to the blur viewport y, with the viewport x as the color texture.
        scene.push_draw_path(path_x);

        // Pop viewport y.
        scene.pop_render_target();

        // This shape goes to the canvas viewport, with the viewport y as the color texture.
        scene.push_draw_path(path_y);
    }

    Canvas::Canvas(const std::shared_ptr<Driver> &p_driver, float size_x, float size_y,
                   const std::vector<char> &area_lut_input) {
        driver = p_driver;

        // Set up a scene.
        scene = std::make_shared<Scene>(0, Rect<float>(0, 0, size_x, size_y));

        // Assign the scene to scene builder.
        scene_builder.scene = scene;

        // Set up a renderer.
#ifndef PATHFINDER_USE_D3D11
        renderer = std::make_shared<RendererD3D9>(p_driver);
#else
        renderer = std::make_shared<RendererD3D11>(p_driver);
#endif

        renderer->set_up(area_lut_input);

        renderer->set_up_pipelines(size_x, size_y);
    }

    void Canvas::push_shape(Shape p_shape,
                            ShapeOp p_shape_op,
                            FillRule p_fill_rule) {
        // Set paint.
        Paint paint = p_shape_op == ShapeOp::Fill ? fill_paint() : stroke_paint();

        // Push to the scene's palette.
        auto paint_id = scene->push_paint(paint);

        auto transform = current_state.transform;
        auto blend_mode = current_state.global_composite_operation;

        p_shape.transform(transform);

        // Add shadow.
        if (!current_state.shadow_color.is_transparent()) {
            // Copy shape.
            auto shadow_shape = p_shape;

            // Set shadow offset.
            shadow_shape.transform(Transform2::from_translation(current_state.shadow_offset));

            auto shadow_blur_info = push_shadow_blur_render_targets(driver, *scene, current_state, shadow_shape.bounds);

            shadow_shape.transform(Transform2::from_translation(-shadow_blur_info.bounds.origin().to_f32()));

            // Per spec the shadow must respect the alpha of the shadowed path, but otherwise have
            // the color of the shadow paint.
            auto shadow_paint = paint;
            auto shadow_base_alpha = shadow_paint.get_base_color().a;
            auto shadow_color = current_state.shadow_color.to_f32();
            shadow_color.a = shadow_color.a * (float) shadow_base_alpha / 255.f;
            shadow_paint.set_base_color(ColorU(shadow_color));

            auto overlay = shadow_paint.get_overlay();
            if (overlay) {
                overlay->composite_op = PaintCompositeOp::DestIn;
            }

            auto shadow_paint_id = scene->push_paint(shadow_paint);

            shadow_shape.paint = shadow_paint_id;
            shadow_shape.fill_rule = p_shape.fill_rule;
            shadow_shape.blend_mode = p_shape.blend_mode;

            // This shape goes to the blur viewport x.
            scene->push_draw_path(shadow_shape);

            composite_shadow_blur_render_targets(*scene, shadow_blur_info);
        }

        p_shape.paint = paint_id;
        p_shape.fill_rule = p_fill_rule;
        p_shape.blend_mode = blend_mode;

        scene->push_draw_path(p_shape);
    }

    void Canvas::fill_shape(Shape p_shape,
                            FillRule p_fill_rule) {
        if (current_state.fill_paint.is_opaque()) {
            push_shape(std::move(p_shape), ShapeOp::Fill, p_fill_rule);
        }
    }

    void Canvas::stroke_shape(Shape p_shape) {
        // Set stroke style.
        auto style = StrokeStyle();
        style.line_width = line_width();
        style.line_join = line_join();
        style.miter_limit = miter_limit();
        style.line_cap = line_cap();

        // Set stroke color.
        auto color = current_state.stroke_paint.get_base_color();

        // No need to draw an invisible stroke.
        if (current_state.stroke_paint.is_opaque() && style.line_width > 0) {
            // Do dash before converting stroke to fill.
            if (!current_state.line_dash.empty()) {
                auto dasher = OutlineDash(p_shape, current_state.line_dash, 0);
                dasher.dash();
                p_shape = dasher.into_outline();
            }

            auto stroke_to_fill = ShapeStrokeToFill(p_shape, style);

            // Do stroking.
            stroke_to_fill.offset();

            auto stroke_shape = stroke_to_fill.into_shape();

            // Strokes don't have the Even-Odd fill rule.
            stroke_shape.fill_rule = FillRule::Winding;

            set_stroke_paint(Paint::from_color(color));

            push_shape(stroke_shape, ShapeOp::Stroke, FillRule::Winding);
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

    void Canvas::build_and_render() {
        scene_builder.build();
        renderer->draw(scene_builder);
    }

    void Canvas::clear() {
        // Create a new scene.
        scene = std::make_shared<Scene>(scene->id, scene->view_box);

        // Update scene for scene builder.
        scene_builder.scene = scene;
    }

    void Canvas::resize(float p_size_x, float p_size_y) {
        renderer->resize_dest_texture(p_size_x, p_size_y);
    }

    std::shared_ptr<Scene> Canvas::get_scene() const {
        return scene;
    }

    std::shared_ptr<Texture> Canvas::get_dest_texture() {
        return renderer->get_dest_texture();
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

    void Canvas::load_svg(const std::string &input) {
        Timestamp timestamp;

        // Load the SVG image via NanoSVG.
        NSVGimage *image;

        // Make a copy as nsvgParse() will empty the string.
        auto input_copy = input;
        char *string_c = const_cast<char *>(input_copy.c_str());
        image = nsvgParse(string_c, "px", 96);

        // Check if image loading is successful.
        if (image == nullptr) {
            throw std::runtime_error(std::string("NanoSVG loading image failed."));
        }

        char buffer[100];
        snprintf(buffer, sizeof(buffer), "SVG image size: (%.1f, %.1f)", image->width, image->height);
        Logger::info(std::string(buffer), "load_svg_from_file");

        timestamp.record("load nsvg image from file");

        // Extract shapes, paths and points from the SVG image.
        for (NSVGshape *nsvg_shape = image->shapes; nsvg_shape != nullptr; nsvg_shape = nsvg_shape->next) {
            Shape shape;

            // Load the bounds from the SVG file, will be modified when pushing points anyway.
            shape.bounds = Rect<float>(nsvg_shape->bounds[0],
                                       nsvg_shape->bounds[1],
                                       nsvg_shape->bounds[2],
                                       nsvg_shape->bounds[3]);

            for (NSVGpath *nsvg_path = nsvg_shape->paths; nsvg_path != nullptr; nsvg_path = nsvg_path->next) {
                shape.move_to(nsvg_path->pts[0], nsvg_path->pts[1]);

                // -6 or -3, both will do, probably.
                for (int point_index = 0; point_index < nsvg_path->npts - 3; point_index += 3) {
                    // * 2 because a point has x and y components.
                    float *p = &nsvg_path->pts[point_index * 2];
                    shape.cubic_to(p[2], p[3], p[4], p[5], p[6], p[7]);
                }

                if (nsvg_path->closed)
                    shape.close();
            }

            // Shadow test.
//            set_shadow_color(ColorU::green());
//            set_shadow_blur(8);

            // Set dash.
            set_line_dash_offset(nsvg_shape->strokeDashOffset);
            set_line_dash(std::vector<float>(nsvg_shape->strokeDashArray,
                                             nsvg_shape->strokeDashArray + nsvg_shape->strokeDashCount));

            // Add fill.
            set_fill_paint(Paint::from_color(ColorU(nsvg_shape->fill.color)));
            auto fill_rule = convert_nsvg_fill_rule(nsvg_shape->fillRule);
            fill_shape(shape, fill_rule);

            // Add stroke if needed.
            set_stroke_paint(Paint::from_color(ColorU(nsvg_shape->stroke.color)));
            set_line_join(convert_nsvg_line_join(nsvg_shape->strokeLineJoin));
            set_miter_limit(nsvg_shape->miterLimit);
            set_line_cap(convert_nsvg_line_cap(nsvg_shape->strokeLineCap));
            set_line_width(nsvg_shape->strokeWidth);
            stroke_shape(shape);
        }

        // Clean up NanoSVG.
        nsvgDelete(image);

        timestamp.record("add shape to canvas");
        timestamp.print();
    }

    void Canvas::draw_image() {

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
}
