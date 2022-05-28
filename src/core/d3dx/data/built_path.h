#ifndef PATHFINDER_BUILT_PATH_H
#define PATHFINDER_BUILT_PATH_H

#include "data.h"
#include "dense_tile_map.h"
#include "../../d3d9/data/gpu_data.h"
#include "../../../common/math/rect.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace Pathfinder {
    struct BuiltShapeData {
        std::vector<int32_t> backdrops;
        DenseTileMap<TileObjectPrimitive> tiles;
    };

    struct BuiltShape {
        BuiltShapeData data;
        Rect<int> tile_bounds;
        FillRule fill_rule = FillRule::Winding;

        uint8_t ctrl_byte = 0;
        uint16_t paint_id = 0;

        /// Path is shape.
        BuiltShape() = default;

        BuiltShape(uint32_t shape_id, Rect<float> path_bounds, uint32_t p_paint_id,
                   Rect<float> view_box_bounds, FillRule p_fill_rule)
                : fill_rule(p_fill_rule), paint_id(p_paint_id) {
            // Set fill rule.
            ctrl_byte = fill_rule == FillRule::EvenOdd ?
                        TILE_CTRL_MASK_EVEN_ODD << TILE_CTRL_MASK_0_SHIFT :
                        TILE_CTRL_MASK_WINDING << TILE_CTRL_MASK_0_SHIFT;

            Rect<float> tile_map_bounds = path_bounds;

            tile_bounds = round_rect_out_to_tile_bounds(tile_map_bounds);

            data.backdrops = std::vector<int32_t>(tile_bounds.width(), 0);

            data.tiles = DenseTileMap<TileObjectPrimitive>(tile_bounds, shape_id, paint_id, ctrl_byte);
        }
    };

    /// This stores a built shape with extra info related to its drawing.
    struct BuiltDrawShape {
    public:
        BuiltDrawShape() = default;

        BuiltDrawShape(BuiltShape p_shape, BlendMode p_blend_mode, FillRule p_fill_rule, bool p_occludes)
                : shape(std::move(p_shape)), blend_mode(p_blend_mode),
                  mask_0_fill_rule(p_fill_rule), occludes(p_occludes) {}

        BuiltShape shape;
        BlendMode blend_mode = BlendMode::SrcIn;
        FillRule mask_0_fill_rule = FillRule::Winding;

        /// If the path is opaque. Used to determine z buffer.
        bool occludes = true;
    };
}

#endif //PATHFINDER_BUILT_PATH_H
