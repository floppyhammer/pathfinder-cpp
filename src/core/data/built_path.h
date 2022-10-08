#ifndef PATHFINDER_BUILT_PATH_H
#define PATHFINDER_BUILT_PATH_H

#include <cstdint>
#include <utility>
#include <vector>

#include "../../common/math/rect.h"
#include "../d3d9/data/gpu_data.h"
#include "data.h"
#include "dense_tile_map.h"

namespace Pathfinder {
struct BuiltPathData {
    /// During tiling, or if backdrop computation is done on GPU, this stores the sum of backdrops
    /// for tile columns above the viewport.
    std::vector<int32_t> backdrops;
    DenseTileMap<TileObjectPrimitive> tiles;
};

struct BuiltPath {
    BuiltPathData data;
    Rect<int> tile_bounds;
    FillRule fill_rule = FillRule::Winding;

    uint8_t ctrl_byte = 0;
    uint16_t paint_id = 0;

    BuiltPath() = default;

    BuiltPath(uint32_t path_id,
              Rect<float> path_bounds,
              uint32_t paint_id,
              Rect<float> view_box_bounds,
              FillRule p_fill_rule);
};

/// This stores a built path with extra info related to its drawing.
struct BuiltDrawPath {
public:
    BuiltDrawPath() = default;

    BuiltDrawPath(BuiltPath p_path, BlendMode p_blend_mode, FillRule p_fill_rule, bool p_occludes)
        : path(std::move(p_path)), blend_mode(p_blend_mode), mask_0_fill_rule(p_fill_rule), occludes(p_occludes) {}

    BuiltPath path;
    BlendMode blend_mode = BlendMode::SrcIn;
    FillRule mask_0_fill_rule = FillRule::Winding;

    /// If the path is opaque. Used to determine z buffer.
    bool occludes = true;
};
} // namespace Pathfinder

#endif // PATHFINDER_BUILT_PATH_H
