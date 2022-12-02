#ifndef PATHFINDER_D3D9_TILER_H
#define PATHFINDER_D3D9_TILER_H

#include "../data/data.h"
#include "../data/path.h"
#include "object_builder.h"

namespace Pathfinder {

/// This might be the most important class for building on D3D9 level.
/// One tiler for one outline.
struct Tiler {
public:
    Tiler(SceneBuilderD3D9& _scene_builder,
          uint32_t path_id,
          const Outline& _outline,
          FillRule fill_rule,
          const RectF& view_box,
          const std::shared_ptr<uint32_t>& clip_path_id,
          const std::vector<BuiltPath>& built_clip_paths,
          TilingPathInfo path_info);

    ObjectBuilder object_builder;

    /// Generate fills and tiles.
    void generate_tiles();

private:
    SceneBuilderD3D9& scene_builder;

    Outline outline;

    std::shared_ptr<BuiltPath> clip_path; // Optional

    /// Process all paths of the attached shape.
    void generate_fills();

    /// Prepare the winding (backdrops) vector for solid tiles.
    void prepare_tiles();
};

} // namespace Pathfinder

#endif // PATHFINDER_D3D9_TILER_H
