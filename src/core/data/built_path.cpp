#include "built_path.h"

#include "data.h"
#include "path.h"

namespace Pathfinder {

BuiltPath::BuiltPath(uint32_t path_id,
                     Rect<float> path_bounds,
                     Rect<float> view_box_bounds,
                     FillRule p_fill_rule,
                     std::shared_ptr<uint32_t> clip_path_id,
                     const TilingPathInfo &path_info)
    : fill_rule(p_fill_rule) {
    if (path_info.type == TilingPathInfo::Type::Draw) {
        paint_id = path_info.info.paint_id;
    }

    ctrl_byte = path_info.to_ctrl();

    Rect<float> tile_map_bounds = path_bounds;

    tile_bounds = round_rect_out_to_tile_bounds(tile_map_bounds);

    data.backdrops = std::vector<int32_t>(tile_bounds.width(), 0);

    data.tiles = DenseTileMap<TileObjectPrimitive>(tile_bounds, path_id, paint_id, ctrl_byte);
}

} // namespace Pathfinder
