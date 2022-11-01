#include "built_path.h"

#include "data.h"
#include "path.h"

namespace Pathfinder {

BuiltPath::BuiltPath(uint32_t path_id,
                     RectF path_bounds,
                     RectF view_box_bounds,
                     FillRule p_fill_rule,
                     const std::shared_ptr<uint32_t> &clip_path_id,
                     const TilingPathInfo &tiling_path_info)
    : fill_rule(p_fill_rule) {
    if (tiling_path_info.type == TilingPathInfo::Type::Draw) {
        paint_id = tiling_path_info.info.paint_id;
    }

    ctrl_byte = tiling_path_info.to_ctrl();

    RectF tile_map_bounds = path_bounds;

    tile_bounds = round_rect_out_to_tile_bounds(tile_map_bounds);

    data.backdrops = std::vector<int32_t>(tile_bounds.width(), 0);

    data.tiles = DenseTileMap<TileObjectPrimitive>(tile_bounds, path_id, paint_id, ctrl_byte);

    if (tiling_path_info.type == TilingPathInfo::Type::Draw) {
        if (clip_path_id) {
            data.clip_tiles = std::make_shared<DenseTileMap<Clip>>(tile_bounds, AlphaTileId(), 0, AlphaTileId(), 0);
        }
    }
}

BuiltDrawPath::BuiltDrawPath(const BuiltPath &built_path, const DrawPath &path_object, const PaintMetadata &paint_metadata) {
    blend_mode = path_object.blend_mode;
    occludes = paint_metadata.is_opaque && blend_mode_occludes_backdrop(blend_mode);

    path = built_path;
    clip_path_id = path_object.clip_path;
    //                filter = paint_metadata.filter();
    color_texture = paint_metadata.tile_batch_texture();
    sampling_flags = TextureSamplingFlags();
    mask_fill_rule = path_object.fill_rule;
}

} // namespace Pathfinder
