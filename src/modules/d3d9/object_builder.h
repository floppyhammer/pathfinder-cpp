//
// Created by chy on 6/15/2021.
//

#ifndef PATHFINDER_D3D9_OBJECT_BUILDER_H
#define PATHFINDER_D3D9_OBJECT_BUILDER_H

#include "../d3d9_d3d11/data/built_path.h"
#include "data/gpu_data.h"
#include "scene_builder.h"

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

        void add_fill(SceneBuilderD3D9 &scene_builder, LineSegmentF p_segment, Vec2<int> tile_coords);

        void adjust_alpha_tile_backdrop(const Vec2<int> &tile_coords, int8_t delta);

        int tile_coords_to_local_index_unchecked(const Vec2<int> &coords) const;

        AlphaTileId get_or_allocate_alpha_tile_index(SceneBuilderD3D9 &scene_builder, const Vec2<int> &tile_coords);
    };
}

#endif //PATHFINDER_D3D9_OBJECT_BUILDER_H
