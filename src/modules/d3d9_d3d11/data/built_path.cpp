//
// Created by chy on 7/9/2021.
//

#include "built_path.h"

#include "data.h"
#include "shape.h"
#include "../paint.h"

namespace Pathfinder {
    BuiltPath::BuiltPath(uint32_t path_id, Rect<float> path_bounds, uint32_t p_paint_id,
                         Rect<float> view_box_bounds, FillRule p_fill_rule)
            : fill_rule(p_fill_rule), paint_id(p_paint_id) {
        // Set fill rule.
        ctrl_byte = fill_rule == FillRule::EvenOdd ?
                    TILE_CTRL_MASK_EVEN_ODD << TILE_CTRL_MASK_0_SHIFT : TILE_CTRL_MASK_WINDING << TILE_CTRL_MASK_0_SHIFT;

        Rect<float> tile_map_bounds;

        tile_map_bounds = path_bounds;

        tile_bounds = round_rect_out_to_tile_bounds(tile_map_bounds);

        data.backdrops = std::vector<int32_t>(tile_bounds.width(), 0);

        data.tiles = DenseTileMap<TileObjectPrimitive>(tile_bounds, path_id, paint_id, ctrl_byte);
    }
}
