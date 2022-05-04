#ifndef PATHFINDER_D3D9_OBJECT_BUILDER_H
#define PATHFINDER_D3D9_OBJECT_BUILDER_H

#include "data/gpu_data.h"
#include "scene_builder.h"
#include "../d3dx/data/built_path.h"

#include <limits>

namespace Pathfinder {
    class ObjectBuilder {
    public:
        BuiltPath built_path;
        std::vector<Fill> fills;
        Rect<float> bounds;

        ObjectBuilder() = default;

        ObjectBuilder(uint32_t path_id, Rect<float> path_bounds, uint32_t paint_id,
                      Rect<float> view_box_bounds, FillRule fill_rule);

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
}

#endif //PATHFINDER_D3D9_OBJECT_BUILDER_H
