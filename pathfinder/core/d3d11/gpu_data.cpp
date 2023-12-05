#include "gpu_data.h"

#ifdef PATHFINDER_ENABLE_D3D11

namespace Pathfinder {

void init_backdrops(std::vector<BackdropInfoD3D11> &backdrops, const uint32_t path_index, const RectI &tile_rect) {
    // Doing reserve might decrease performance.
    // backdrops.reserve(backdrops.size() + tile_rect.width());

    for (int32_t tile_x_offset = 0; tile_x_offset < tile_rect.width(); tile_x_offset++) {
        backdrops.push_back({0, tile_x_offset, path_index});
    }
}

TileBatchDataD3D11::TileBatchDataD3D11(uint32_t p_batch_id, PathSource p_path_source) {
    batch_id = p_batch_id;

    path_source = p_path_source;

    prepare_info = PrepareTilesInfoD3D11{};
}

uint32_t TileBatchDataD3D11::push(const BuiltPath &path,
                                  uint32_t global_path_id,
                                  const std::shared_ptr<GlobalPathId> &batch_clip_path_id,
                                  bool z_write,
                                  LastSceneInfo &last_scene) {
    auto batch_path_index = path_count;
    path_count++;

    prepare_info.propagate_metadata.push_back({path.tile_bounds,
                                               tile_count,
                                               batch_path_index,
                                               z_write,
                                               batch_clip_path_id ? batch_clip_path_id->path_index : ~0u,
                                               static_cast<uint32_t>(prepare_info.backdrops.size())});

    init_backdrops(prepare_info.backdrops, batch_path_index, path.tile_bounds);

    auto &segment_ranges =
        path_source == PathSource::Draw ? last_scene.draw_segment_ranges : last_scene.clip_segment_ranges;

    auto &segment_range = segment_ranges[global_path_id];

    prepare_info.dice_metadata.push_back({global_path_id, static_cast<uint32_t>(segment_range.start), segment_count});

    prepare_info.tile_path_info.push_back({static_cast<int16_t>(path.tile_bounds.min_x()),
                                           static_cast<int16_t>(path.tile_bounds.min_y()),
                                           static_cast<int16_t>(path.tile_bounds.max_x()),
                                           static_cast<int16_t>(path.tile_bounds.max_y()),
                                           tile_count,
                                           path.paint_id,
                                           path.ctrl_byte,
                                           0});

    tile_count += path.tile_bounds.area();
    segment_count += segment_range.end - segment_range.start;

    // Handle clip.

    uint32_t clip_batch_id;
    if (batch_clip_path_id) {
        clip_batch_id = batch_clip_path_id->batch_id;
    } else {
        return batch_path_index;
    }

    if (clipped_path_info == nullptr) {
        clipped_path_info = std::make_shared<ClippedPathInfo>(ClippedPathInfo{clip_batch_id, 0, 0, nullptr});
    }

    clipped_path_info->clipped_path_count += 1;
    clipped_path_info->max_clipped_tile_count += path.tile_bounds.area();

    return batch_path_index;
}

Range SegmentsD3D11::add_path(const Outline &outline) {
    auto first_segment_index = indices.size();

    // Traverse all contours in the outline.
    for (auto &contour : outline.contours) {
        auto point_count = contour.points.size();

        // Traverse all points in the contour.
        for (size_t point_index = 0; point_index < point_count; point_index++) {
            // If this point is an on-curve point.
            if (contour.flags[point_index] == PointFlag::ON_CURVE_POINT) {
                // Segment type. Default is a line.
                uint32_t flag = 0;

                // If the next point is a 0-type control point.
                if (point_index + 1 < point_count && contour.flags[point_index + 1] == PointFlag::CONTROL_POINT_0) {
                    // If the point behind the next point is a 1-type control point, this is a cubic curve.
                    if (point_index + 2 < point_count && contour.flags[point_index + 2] == PointFlag::CONTROL_POINT_1) {
                        flag = CURVE_IS_CUBIC;
                    } else { // This is a quadratic curve.
                        flag = CURVE_IS_QUADRATIC;
                    }
                }

                indices.push_back({static_cast<uint32_t>(points.size()), flag});
            }

            points.push_back(contour.points[point_index]);
        }

        points.push_back(contour.points[0]);
    }

    auto last_segment_index = indices.size();

    return {first_segment_index, last_segment_index};
}

} // namespace Pathfinder

#endif
