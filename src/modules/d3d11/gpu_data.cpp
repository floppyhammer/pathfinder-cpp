//
// Created by floppyhammer on 2021/11/4.
//

#include "gpu_data.h"

#ifdef PATHFINDER_USE_D3D11

namespace Pathfinder {
    void init_backdrops(std::vector<BackdropInfoD3D11> &backdrops,
                        const uint32_t path_index,
                        const Rect<int> &tile_rect) {
        // Doing reserve might decrease performance.
        //backdrops.reserve(backdrops.size() + tile_rect.width());

        for (int32_t tile_x_offset = 0; tile_x_offset < tile_rect.width(); tile_x_offset++) {
            backdrops.emplace_back(
                    0,
                    tile_x_offset,
                    path_index
            );
        }
    }

    TileBatchDataD3D11::TileBatchDataD3D11(uint32_t p_batch_id,
                                           PathSource p_path_source) {
        batch_id = p_batch_id;

        path_source = p_path_source;

        prepare_info = PrepareTilesInfoD3D11{};
    }

    uint32_t TileBatchDataD3D11::push(BuiltPath &path,
                                      uint32_t global_path_id,
                                      bool z_write,
                                      LastSceneInfo &last_scene) {
        auto batch_path_index = path_count;
        path_count++;

        prepare_info.propagate_metadata.emplace_back(
                path.tile_bounds,
                tile_count,
                batch_path_index,
                z_write,
                static_cast<uint32_t>(~0),
                static_cast<uint32_t>(prepare_info.backdrops.size())
        );

        init_backdrops(prepare_info.backdrops, batch_path_index, path.tile_bounds);

        auto &segment_ranges = last_scene.draw_segment_ranges;
        auto &segment_range = segment_ranges[global_path_id];

        prepare_info.dice_metadata.emplace_back(
                global_path_id,
                static_cast<uint32_t>(segment_range.start),
                segment_count
        );

        prepare_info.tile_path_info.emplace_back(
                static_cast<int16_t>(path.tile_bounds.min_x()),
                static_cast<int16_t>(path.tile_bounds.min_y()),
                static_cast<int16_t>(path.tile_bounds.max_x()),
                static_cast<int16_t>(path.tile_bounds.max_y()),
                tile_count,
                path.paint_id,
                path.ctrl_byte,
                0
        );

        tile_count += path.tile_bounds.area();
        segment_count += segment_range.end - segment_range.start;

        return batch_path_index;
    }

    Range SegmentsD3D11::add_path(const Shape &shape) {
        auto first_segment_index = indices.size();

        // Traverse all paths in the shape.
        for (auto &path: shape.paths) {
            auto point_count = path.points.size();

            // Traverse all points in the path.
            for (size_t point_index = 0; point_index < point_count; point_index++) {
                // If this point is an on-curve point.
                if ((path.flags[point_index].value
                     & (PointFlags::CONTROL_POINT_0 | PointFlags::CONTROL_POINT_1)) == 0) {
                    // Segment type. Default is a line.
                    uint32_t flag = 0;

                    // If the next point is a 0-type control point.
                    if (point_index + 1 < point_count &&
                        (path.flags[point_index + 1].value & PointFlags::CONTROL_POINT_0) != 0) {
                        // If the point behind the next point is a 1-type control point, this is a cubic curve.
                        if (point_index + 2 < point_count &&
                            (path.flags[point_index + 2].value & PointFlags::CONTROL_POINT_1) != 0) {
                            flag = CURVE_IS_CUBIC;
                        } else { // This is a quadratic curve.
                            flag = CURVE_IS_QUADRATIC;
                        }
                    }

                    indices.emplace_back(static_cast<uint32_t>(points.size()), flag);
                }

                points.push_back(path.points[point_index]);
            }

            points.push_back(path.points[0]);
        }

        auto last_segment_index = indices.size();

        return {first_segment_index, last_segment_index};
    }
}

#endif
