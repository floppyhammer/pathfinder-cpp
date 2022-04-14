//
// Created by floppyhammer on 6/24/2021.
//

#include "object_builder.h"

#include "../d3dx/data/data.h"
#include "../../common/f32x4.h"
#include "../../common/global_macros.h"
#include "../../common/math/basic.h"

namespace Pathfinder {
    ObjectBuilder::ObjectBuilder(uint32_t path_id, Rect<float> path_bounds,
                                 uint32_t paint_id, Rect<float> view_box_bounds,
                                 FillRule fill_rule) : bounds(path_bounds) {
        built_path = BuiltPath(path_id, path_bounds, paint_id, view_box_bounds, fill_rule);
    }

    void ObjectBuilder::add_fill(SceneBuilderD3D9 &scene_builder, LineSegmentF p_segment, Vec2<int> tile_coords) {
        // Ensure this fill is in bounds. If not, cull it.
        if (!built_path.tile_bounds.contains_point(tile_coords)) {
            return;
        }

        // Compute the upper left corner of the tile.
        auto tile_size = F32x4::splat(TILE_WIDTH);
        auto tile_upper_left = F32x4(tile_coords.to_float(), Vec2<float>()).xyxy() * tile_size;

        // To tile's local coordinates.
        F32x4 segment = (p_segment.value - tile_upper_left) * F32x4::splat(256.0);

        // Clamp the segment within the tile.
        F32x4 min = F32x4::splat(0.0);
        F32x4 max = F32x4::splat(TILE_WIDTH * 256 - 1);
        segment = segment.clamp(min, max);

        // FIXME: This is disabled for the sake of performance.
        //segment = segment.round();

        auto from_x = static_cast<uint16_t>(segment.get<0>());
        auto from_y = static_cast<uint16_t>(segment.get<1>());
        auto to_x = static_cast<uint16_t>(segment.get<2>());
        auto to_y = static_cast<uint16_t>(segment.get<3>());

        // Handle vertical segments. Cull degenerate fills.
        if (from_x == to_x) {
            return;
        }

        // Get the alpha tile id of this tile coordinates.
        // Allocate a new alpha tile if necessary.
        auto alpha_tile_id = get_or_allocate_alpha_tile_index(scene_builder, tile_coords);

        // Reserve some space beforehand, so we don't need to allocate every time we push a new fill.
        if (fills.capacity() - fills.size() <= 0) {
            fills.reserve(fills.size() + 4096);
        }

        // Add a fill.
        fills.push_back(Fill{
                LineSegmentU16{from_x, from_y, to_x, to_y},
                alpha_tile_id.value,
        });
    }

    int ObjectBuilder::tile_coords_to_local_index_unchecked(const Vec2<int> &coords) const {
        auto tile_rect = built_path.tile_bounds;
        auto offset = coords - tile_rect.origin();
        return offset.x + tile_rect.width() * offset.y;
    }

    AlphaTileId ObjectBuilder::get_or_allocate_alpha_tile_index(SceneBuilderD3D9 &scene_builder,
                                                                const Vec2<int> &tile_coords) {
        // Tile index in the tile bounds.
        auto local_tile_index = tile_coords_to_local_index_unchecked(tile_coords);

        // Get a reference of the dense tile map from the built path data.
        auto &tiles = built_path.data.tiles;

        // Get the alpha tile id.
        auto alpha_tile_id = tiles.data[local_tile_index].alpha_tile_id;

        // If the alpha tile id is valid, return it.
        if (alpha_tile_id.is_valid()) {
            return alpha_tile_id;
        }

        // Else, allocate a new alpha tile id.
        alpha_tile_id = AlphaTileId(scene_builder.next_alpha_tile_indices, 0);

        // Assign the new id.
        tiles.data[local_tile_index].alpha_tile_id = alpha_tile_id;

        return alpha_tile_id;
    }

    void ObjectBuilder::adjust_alpha_tile_backdrop(const Vec2<int> &tile_coords, int8_t delta) {
        auto &tiles = built_path.data.tiles;
        auto &backdrops = built_path.data.backdrops;

        auto tile_offset = tile_coords - tiles.rect.origin();

        // Invalid tile.
        if (tile_offset.x < 0 || tile_offset.x >= tiles.rect.width() || tile_offset.y >= tiles.rect.height()) {
            return;
        }

        if (tile_offset.y < 0) {
            backdrops[tile_offset.x] += delta;
            return;
        }

        auto local_tile_index = tiles.coords_to_index_unchecked(tile_coords);
        tiles.data[local_tile_index].backdrop += delta;
    }
}
