#ifndef PATHFINDER_D3D9_OBJECT_BUILDER_H
#define PATHFINDER_D3D9_OBJECT_BUILDER_H

#include <limits>

#include "../data/built_path.h"
#include "data/gpu_data.h"
#include "scene_builder.h"

namespace Pathfinder {

class ObjectBuilder {
public:
    BuiltPath built_path;
    std::vector<Fill> fills;
    RectF bounds;

    ObjectBuilder() = default;

    ObjectBuilder(uint32_t path_id,
                  RectF path_bounds,
                  const RectF &view_box_bounds,
                  FillRule fill_rule,
                  const std::shared_ptr<uint32_t> &clip_path_id,
                  const TilingPathInfo &path_info);

    /// Alpha tile id is set at this stage.
    void add_fill(SceneBuilderD3D9 &scene_builder, LineSegmentF p_segment, Vec2<int> tile_coords);

    void adjust_alpha_tile_backdrop(const Vec2<int> &tile_coords, int8_t delta);

    int tile_coords_to_local_index_unchecked(const Vec2<int> &coords) const;

    /**
     * Get the alpha tile by tile coordinates, and allocate one if there's none.
     * @param scene_builder
     * @param tile_coords
     * @return Alpha tile ID.
     */
    AlphaTileId get_or_allocate_alpha_tile_index(SceneBuilderD3D9 &scene_builder, const Vec2<int> &tile_coords);
};
} // namespace Pathfinder

#endif // PATHFINDER_D3D9_OBJECT_BUILDER_H
