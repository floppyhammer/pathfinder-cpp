#include "svg.h"

#include "../common/io.h"

#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>

namespace Pathfinder {

/**
 * @brief Convert NanoSVG fill rule enum to our own type.
 * @param line_cap NanoSVG fill rule enum
 * @return FillRule enum
 */
FillRule convert_nsvg_fill_rule(char fill_rule) {
    return fill_rule == NSVGfillRule::NSVG_FILLRULE_EVENODD ? FillRule::EvenOdd : FillRule::Winding;
}

/**
 * @brief Convert NanoSVG line cap enum to our own type.
 * @param line_cap NanoSVG line cap enum
 * @return LineCap enum
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
 * @param line_cap NanoSVG line join enum
 * @return LineJoin enum
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

/**
 * @brief Convert NanoSVG paint to our own type.
 * @param nsvg_paint NanoSVG paint
 * @return Paint
 */
Paint convert_nsvg_paint(NSVGpaint nsvg_paint) {
    Paint paint;

    // FIXME: Non-Identity transform will cause incorrect gradient (both linear and radical) rendering.
    switch (nsvg_paint.type) {
        case NSVG_PAINT_NONE:
            break;
        case NSVG_PAINT_COLOR: {
            paint = Paint::from_color(ColorU(nsvg_paint.color));
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

SvgScene::SvgScene() {
    scene = std::make_shared<Scene>(0, RectF());
}

void SvgScene::load_from_memory(std::vector<char> bytes, Canvas &canvas) {
    if (bytes.empty()) {
        Logger::error("SVG input is empty!", "SvgScene");
        return;
    }

    // Load as a NanoSVG image.
    NSVGimage *image = nsvgParse(bytes.data(), "px", 96);

    // Check if image loading is successful.
    if (image == nullptr) {
        Logger::error("NanoSVG loading image failed!", "Canvas");
        return;
    }

    auto old_scene = canvas.replace_scene(scene);

    canvas.set_size({(int)image->width, (int)image->height});

    // Extract paths, contours and points from the SVG image.
    // Notable: NSVGshape equals to our Path, and NSVGpath equals to our Contour (Sub-Path).
    for (NSVGshape *nsvg_shape = image->shapes; nsvg_shape != nullptr; nsvg_shape = nsvg_shape->next) {
        Path2d path;

        for (NSVGpath *nsvg_path = nsvg_shape->paths; nsvg_path != nullptr; nsvg_path = nsvg_path->next) {
            path.move_to(nsvg_path->pts[0], nsvg_path->pts[1]);

            for (int point_index = 0; point_index < nsvg_path->npts - 3; point_index += 3) {
                // * 2 because a point has x and y components.
                float *p = &nsvg_path->pts[point_index * 2];

                // Remove duplicate points added by NanoSVG.
                if (p[2] == p[4] && p[4] == p[6] && p[3] == p[5] && p[5] == p[7]) {
                    continue;
                }

                path.cubic_to(p[2], p[3], p[4], p[5], p[6], p[7]);
            }

            if (nsvg_path->closed) {
                path.close_path();
            }
        }

        canvas.save_state();

        // Set dash.
        canvas.set_line_dash_offset(nsvg_shape->strokeDashOffset);
        canvas.set_line_dash(
            std::vector<float>(nsvg_shape->strokeDashArray, nsvg_shape->strokeDashArray + nsvg_shape->strokeDashCount));

        // Add fill.
        canvas.set_fill_paint(convert_nsvg_paint(nsvg_shape->fill));
        canvas.fill_path(path, convert_nsvg_fill_rule(nsvg_shape->fillRule));

        // Add stroke.
        canvas.set_line_join(convert_nsvg_line_join(nsvg_shape->strokeLineJoin));
        canvas.set_miter_limit(nsvg_shape->miterLimit);
        canvas.set_line_cap(convert_nsvg_line_cap(nsvg_shape->strokeLineCap));
        canvas.set_line_width(nsvg_shape->strokeWidth);
        canvas.set_stroke_paint(convert_nsvg_paint(nsvg_shape->stroke));
        canvas.stroke_path(path);

        canvas.restore_state();
    }

    scene = canvas.replace_scene(old_scene);

    // Clean up NanoSVG image.
    nsvgDelete(image);
}

std::shared_ptr<Scene> SvgScene::get_scene() const {
    return scene;
}

} // namespace Pathfinder
