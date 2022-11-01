#ifndef PATHFINDER_BUILT_PATH_H
#define PATHFINDER_BUILT_PATH_H

#include <cstdint>
#include <utility>
#include <vector>

#include "../../common/math/rect.h"
#include "../../gpu/texture.h"
#include "../d3d9/data/gpu_data.h"
#include "../paint/paint.h"
#include "data.h"
#include "dense_tile_map.h"
#include "path.h"

using std::shared_ptr;

namespace Pathfinder {

struct BuiltPathData {
    /// During tiling, or if backdrop computation is done on GPU, this stores the sum of backdrops
    /// for tile columns above the viewport.
    std::vector<int32_t> backdrops;
    DenseTileMap<TileObjectPrimitive> tiles;
    shared_ptr<DenseTileMap<Clip>> clip_tiles;
};

struct BuiltPath {
    BuiltPathData data;
    RectI tile_bounds;
    FillRule fill_rule = FillRule::Winding;
    std::shared_ptr<uint32_t> clip_path_id;
    uint8_t ctrl_byte = 0;
    uint16_t paint_id = 0;

    BuiltPath() = default;

    BuiltPath(uint32_t path_id,
              RectF path_bounds,
              RectF view_box_bounds,
              FillRule p_fill_rule,
              const std::shared_ptr<uint32_t> &clip_path_id,
              const TilingPathInfo &tiling_path_info);
};

/// This stores a built path with extra info related to its drawing.
struct BuiltDrawPath {
    BuiltPath path;
    shared_ptr<uint32_t> clip_path_id;
    BlendMode blend_mode;
    //    Filter filter;
    shared_ptr<Texture> color_texture; // Will be used in tile batch building.
    TextureSamplingFlags sampling_flags;
    FillRule mask_fill_rule;

    /// If the path is opaque. Used to determine z buffer.
    bool occludes = true;

    BuiltDrawPath() = default;
    BuiltDrawPath(const BuiltPath &built_path, const DrawPath &path_object, const PaintMetadata &paint_metadata);
};

} // namespace Pathfinder

#endif // PATHFINDER_BUILT_PATH_H
