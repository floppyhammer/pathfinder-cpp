#include "tiler.h"

#include "../../common/math/basic.h"
#include "../../common/timestamp.h"

#undef min
#undef max

namespace Pathfinder {
    // This value amounts to 16.0 * tolerance * tolerance in Pathfinder Rust.
    const float FLATTENING_TOLERANCE = 1.0f;

    enum class StepDirection {
        None,
        X,
        Y,
    };

    bool Outcode::is_empty() const {
        return flag == 0x00;
    }

    bool Outcode::contains(uint8_t p_side) const {
        return (flag & p_side) != 0x00;
    }

    Outcode compute_outcode(const Vec2<float>& point, const Rect<float>& rect) {
        Outcode outcode;

        if (point.x < rect.min_x()) {
            outcode.flag |= Outcode::LEFT;
        } else if (point.x > rect.max_x()) {
            outcode.flag |= Outcode::RIGHT;
        }

        if (point.y < rect.min_y()) {
            outcode.flag |= Outcode::TOP;
        } else if (point.y > rect.max_y()) {
            outcode.flag |= Outcode::BOTTOM;
        }

        return outcode;
    }

    /// Clips a line segment to an axis-aligned rectangle using Cohen-Sutherland clipping.
    bool clip_line_segment_to_rect(LineSegmentF &line_segment, Rect<float> rect) {
        auto outcode_from = compute_outcode(line_segment.from(), rect);
        auto outcode_to = compute_outcode(line_segment.to(), rect);

        while (true) {
            // The line segment is inside the bounding rect. No clipping.
            if (outcode_from.is_empty() && outcode_to.is_empty())
                return true;

            // The line segment is outside the bounding rect, so no clipping.
            if (!(outcode_from & outcode_to).is_empty())
                return false;

            // The line segment crosses the boundaries of the bounding rect.

            // If we should clip the from point.
            auto clip_from = outcode_from.flag > outcode_to.flag;
            Vec2<float> point{};
            Outcode outcode;

            if (clip_from) {
                point = line_segment.from();
                outcode = outcode_from;
            } else {
                point = line_segment.to();
                outcode = outcode_to;
            }

            if (outcode.contains(Outcode::LEFT)) {
                point = Vec2<float>(rect.min_x(),
                                    lerp(line_segment.from().y,
                                         line_segment.to().y,
                                         (rect.min_x() - line_segment.from().x) /
                                         (line_segment.to().x - line_segment.from().x)));
            } else if (outcode.contains(Outcode::RIGHT)) {
                point = Vec2<float>(rect.max_x(),
                                    lerp(line_segment.from().y,
                                         line_segment.to().y,
                                         (rect.max_x() - line_segment.from().x) /
                                         (line_segment.to().x - line_segment.from().x)));
            } else if (outcode.contains(Outcode::TOP)) {
                point = Vec2<float>(lerp(line_segment.from().x,
                                         line_segment.to().x,
                                         (rect.min_y() - line_segment.from().y) /
                                         (line_segment.to().y - line_segment.from().y)),
                                    rect.min_y());
            } else if (outcode.contains(Outcode::BOTTOM)) {
                point = Vec2<float>(lerp(line_segment.from().x,
                                         line_segment.to().x,
                                         (rect.max_y() - line_segment.from().y) /
                                         (line_segment.to().y - line_segment.from().y)),
                                    rect.max_y());
            }

            if (clip_from) {
                line_segment.set_from(point);
                outcode_from = compute_outcode(point, rect);
            } else {
                line_segment.set_to(point);
                outcode_to = compute_outcode(point, rect);
            }
        }
    }

    void process_line_segment(LineSegmentF p_line_segment,
                              SceneBuilderD3D9 &p_scene_builder,
                              ObjectBuilder &p_object_builder) {
        // Clip the line segment if it intersects the view box bounds.
        {
            // Clip by the view box.
            auto clip_box = p_scene_builder.get_scene()->get_view_box();

            // Clipping doesn't happen to the top bound as the ray goes from that direction.
            clip_box.top = -std::numeric_limits<float>::infinity();

            // Clip the line segment.
            const bool inside_view = clip_line_segment_to_rect(p_line_segment, clip_box);

            // If the line segment falls outside the clip box, no need to process it.
            if (!inside_view)
                return;
        }

        // Tile size.
        const auto tile_size = Vec2<float>(TILE_WIDTH, TILE_HEIGHT);

        F32x4 tile_line_segment = p_line_segment.value * F32x4::splat(1.0f / TILE_WIDTH);

        // Tile the line segment, get the tile coords (index) of the FROM and TO points.
        const auto from_tile_coords = tile_line_segment.xy().floor();
        const auto to_tile_coords = tile_line_segment.zw().floor();

        // Line segment vector.
        const auto vector = p_line_segment.vector();

        // Step is the direction to advance the tile.
        const auto step = Vec2<int>(vector.x < 0 ? -1 : 1, vector.y < 0 ? -1 : 1);

        // Real coordinates of the top left of the first tile crossing.
        // Compute `first_tile_crossing = (from_tile_coords + vec2i(vector.x >= 0 ? 1 : 0, vector.y >= 0 ? 1 : 0)) * tile_size`.
        const auto first_tile_crossing = (from_tile_coords.to_float()
                                          + Vec2<float>(vector.x >= 0 ? 1 : 0, vector.y >= 0 ? 1 : 0)) * tile_size;

        // Value of t at which the ray crosses the first vertical/horizontal tile boundary.
        auto t_max = (first_tile_crossing - p_line_segment.from()) / vector;

        // This indicates how far along the ray we must move
        // (in units of t) for the horizontal/vertical component of such a
        // movement to equal the width/height of a tile.
        const auto t_delta = (tile_size / vector).abs();

        auto current_position = p_line_segment.from();
        auto tile_coords = from_tile_coords;

        // Decide the direction of the next step to take.
        auto last_step_direction = StepDirection::None;

        // Advance to the next tile.
        while (true) {
            // Decide the direction of the next step to take.
            StepDirection next_step_direction;

            if (t_max.x < t_max.y) {
                next_step_direction = StepDirection::X;
            } else if (t_max.x > t_max.y) {
                next_step_direction = StepDirection::Y;
            } else {
                // This should only happen if the line's destination is precisely on a corner point
                // between tiles:
                //
                //     +-----+--O--+
                //     |     | /   |
                //     |     |/    |
                //     +-----O-----+
                //     |     | end |
                //     |     | tile|
                //     +-----+-----+
                //
                // In that case we just need to step in the positive direction to move to the lower
                // right tile.
                if (step.x > 0) {
                    next_step_direction = StepDirection::X;
                } else {
                    next_step_direction = StepDirection::Y;
                }
            }

            float next_t;
            if (next_step_direction == StepDirection::X) {
                next_t = t_max.x;
            } else {
                next_t = t_max.y;
            }
            next_t = std::min(next_t, 1.0f);

            // If we've reached the end tile, don't step at all.
            if (tile_coords == to_tile_coords) {
                next_step_direction = StepDirection::None;
            }

            // Get next position on segment.
            const auto next_position = p_line_segment.sample(next_t);

            const auto clipped_line_segment = LineSegmentF(current_position, next_position);

            // Core step. Add fill.
            p_object_builder.add_fill(p_scene_builder, clipped_line_segment, tile_coords);

            // Add extra fills if necessary.
            // This happens when the segment crosses boundaries vertically, in which we need two quad fills to describe it.
            if (step.y < 0 && next_step_direction == StepDirection::Y) {
                // Leave the current tile through its top boundary.
                const auto auxiliary_segment = LineSegmentF(clipped_line_segment.to(),
                                                            tile_coords.to_float() * tile_size);
                p_object_builder.add_fill(p_scene_builder, auxiliary_segment, tile_coords);
            } else if (step.y > 0 && last_step_direction == StepDirection::Y) {
                // Enter a new tile through its top boundary.
                const auto auxiliary_segment = LineSegmentF(tile_coords.to_float() * tile_size,
                                                            clipped_line_segment.from());
                p_object_builder.add_fill(p_scene_builder, auxiliary_segment, tile_coords);
            }

            // Adjust backdrop (i.e. winding) if necessary.
            if (step.x < 0 && last_step_direction == StepDirection::X) {
                // Enter a new tile through its right boundary.
                p_object_builder.adjust_alpha_tile_backdrop(tile_coords, 1);
            } else if (step.x > 0 && next_step_direction == StepDirection::X) {
                // Leave the current tile through its right boundary.
                p_object_builder.adjust_alpha_tile_backdrop(tile_coords, -1);
            }

            // Take a step.
            if (next_step_direction == StepDirection::X) { // Go horizontally.
                t_max.x += t_delta.x;
                tile_coords.x += step.x;
            } else if (next_step_direction == StepDirection::Y) { // Go vertically.
                t_max.y += t_delta.y;
                tile_coords.y += step.y;
            } else { // Reach end.
                break;
            }

            // Update current position.
            current_position = next_position;

            // Update last step direction.
            last_step_direction = next_step_direction;
        }
    }

    void process_segment(Segment &p_segment, SceneBuilderD3D9 &p_scene_builder, ObjectBuilder &p_object_builder) {
        // TODO(pcwalton): Stop degree elevating.
        // If the segment is a quadratic curve.
        if (p_segment.is_quadratic()) {
            // Convert to a cubic one.
            auto cubic = p_segment.to_cubic();
            process_segment(cubic, p_scene_builder, p_object_builder);

            // Remember to return to avoid running code below.
            return;
        }

        // If the segment is a line or a cubic curve that is flat enough, go to next step.
        if (p_segment.is_line() || (p_segment.is_cubic() && p_segment.is_flat(FLATTENING_TOLERANCE))) {
            // (Next step) Process the segment as a line segment.
            process_line_segment(p_segment.baseline, p_scene_builder, p_object_builder);

            // Remember to return to avoid running code below.
            return;
        }

        // If the segment is a cubic curve.
        Segment prev, next;
        p_segment.split(0.5f, prev, next);

        process_segment(prev, p_scene_builder, p_object_builder);
        process_segment(next, p_scene_builder, p_object_builder);
    }

    Tiler::Tiler(SceneBuilderD3D9 &p_scene_builder, uint32_t path_id, const DrawPath &p_path,
                 FillRule fill_rule, const Rect<float> &view_box)
            : path(p_path), scene_builder(p_scene_builder) {
        // The intersection rect of the path bounds and the view box.
        auto bounds = path.outline.bounds.intersection(view_box);

        // Create an object builder.
        object_builder = ObjectBuilder(path_id, bounds, path.paint, view_box, fill_rule);
    }

    void Tiler::generate_tiles() {
        // Core step. Fills are used for tile masking.
        generate_fills();

        // Prepare winding data for tiles. Winding data for fills are obtained in generate_fills().
        prepare_tiles();
    }

    void Tiler::generate_fills() {
        // Traverse paths in the shape.
        for (const auto &contour: path.outline.contours) {
            auto segments_iter = SegmentsIter(contour.points, contour.flags, contour.closed);

            // Traverse curve/line segments.
            while (!segments_iter.is_at_end()) {
                auto segment = segments_iter.get_next(true);

                // Break for invalid segment.
                if (segment.kind == SegmentKind::None)
                    break;

                process_segment(segment, scene_builder, object_builder);
            }
        }
    }

    void Tiler::prepare_tiles() {
        // Prepared previously by generate_fills().
        auto &backdrops = object_builder.built_path.data.backdrops;
        auto &tiles = object_builder.built_path.data.tiles;

        auto tiles_across = tiles.rect.width();

        // Traverse the tile map.
        // Propagate backdrops row by row.
        for (int draw_tile_index = 0; draw_tile_index < tiles.data.size(); draw_tile_index++) {
            // Current tile.
            auto &draw_tile = tiles.data[draw_tile_index];

            // Current column.
            auto column = draw_tile_index % tiles_across;

            // Local winding change of the tile.
            auto delta = draw_tile.backdrop;

            auto draw_alpha_tile_id = draw_tile.alpha_tile_id;
            auto draw_tile_backdrop = backdrops[column];

            // Suppose to handle clip path in here. Skip for now.

            draw_tile.alpha_tile_id = draw_alpha_tile_id;
            draw_tile.backdrop = (int8_t) draw_tile_backdrop;

            // Add local winding to global.
            backdrops[column] += delta;
        }
    }
}
