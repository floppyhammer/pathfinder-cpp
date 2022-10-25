#ifndef PATHFINDER_DENSE_TILE_MAP_H
#define PATHFINDER_DENSE_TILE_MAP_H

#include <vector>

#include "../../common/color.h"
#include "../../common/math/rect.h"

namespace Pathfinder {

template <typename T>
struct DenseTileMap {
    // It may contain TileObjectPrimitives, z buffer or Clips.
    std::vector<T> data;

    // Tile map region.
    RectI rect;

    DenseTileMap() = default;

    DenseTileMap(const std::vector<T> &p_data, const RectI &p_rect) : data(p_data), rect(p_rect) {}

    /// Constructor for TileObjectPrimitive.
    DenseTileMap(const RectI &p_rect, uint32_t p_path_id, uint32_t p_paint_id, uint8_t p_ctrl_byte) : rect(p_rect) {
        data = std::vector<T>(rect.width() * rect.height(), T());

        for (int y = rect.min_y(); y < rect.max_y(); y++) {
            int offset = (y - rect.min_y()) * rect.width();
            for (int x = rect.min_x(); x < rect.max_x(); x++) {
                int index = offset + (x - rect.min_x());
                data[index].tile_x = x;
                data[index].tile_y = y;
                data[index].ctrl = p_ctrl_byte;
                data[index].path_id = p_path_id;
                data[index].metadata_id = p_paint_id;
            }
        }
    }

    /// Constructor for Clip.
    DenseTileMap(const RectI &p_rect,
                 AlphaTileId dest_tile_id,
                 int32_t dest_backdrop,
                 AlphaTileId src_tile_id,
                 int32_t src_backdrop)
        : rect(p_rect) {
        data = std::vector<T>(rect.width() * rect.height(), T());

        for (int y = rect.min_y(); y < rect.max_y(); y++) {
            int offset = (y - rect.min_y()) * rect.width();
            for (int x = rect.min_x(); x < rect.max_x(); x++) {
                int index = offset + (x - rect.min_x());
                data[index].dest_tile_id = dest_tile_id;
                data[index].dest_backdrop = dest_backdrop;
                data[index].src_tile_id = src_tile_id;
                data[index].src_backdrop = src_backdrop;
            }
        }
    }

    /// A quick way to build z buffer.
    static inline DenseTileMap z_builder(const RectI &p_rect) {
        return {std::vector<T>(p_rect.width() * p_rect.height(), 0), p_rect};
    }

    inline T *get(const Vec2I &coords) {
        auto index = coords_to_index(coords);

        // We have to make sure the index we get is valid.
        if (index) {
            return &data[*index];
        } else {
            return nullptr;
        }
    }

    /// A safe call to find index by coordinates.
    inline std::shared_ptr<size_t> coords_to_index(const Vec2I &coords) {
        if (rect.contains_point(coords)) {
            return std::make_shared<size_t>(coords_to_index_unchecked(coords));
        } else {
            return nullptr;
        }
    }

    /// An unsafe call to index by coordinates.
    /// The tile map's top and bottom bounds are not considered.
    inline int coords_to_index_unchecked(const Vec2I &coords) {
        return (coords.y - rect.min_y()) * rect.size().x + coords.x - rect.min_x();
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_DENSE_TILE_MAP_H
