#include "built_path.h"

#include "data.h"
#include "path.h"

namespace Pathfinder {

BuiltPath::BuiltPath(uint32_t path_id,
                     RectF path_bounds,
                     RectF view_box_bounds,
                     FillRule p_fill_rule,
                     const std::shared_ptr<uint32_t>& clip_path_id,
                     const TilingPathInfo& tiling_path_info)
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

} // namespace Pathfinder
