//
// Created by floppyhammer on 2021/11/3.
//

#ifndef PATHFINDER_D3D9_GPU_DATA_H
#define PATHFINDER_D3D9_GPU_DATA_H

#include "line_segment.h"
#include "alpha_tile_id.h"

namespace Pathfinder {
    /// These data will be sent to GPU directly.

    /// A vector of this will be sent to the fill program.
    struct Fill {
        LineSegmentU16 line_segment;

        /// Index of the alpha tile that this fill belongs to.
        uint32_t link;
    };

    /// A vector of this will be sent the tile program.
    struct TileObjectPrimitive {
        int16_t tile_x = 0;
        int16_t tile_y = 0;
        AlphaTileId alpha_tile_id; // 4 bytes
        uint8_t ctrl = 0;
        int8_t backdrop = 0;
        uint32_t path_id = 0;
        uint32_t color = 0;

        TileObjectPrimitive() = default;

        TileObjectPrimitive(int16_t p_tile_x,
                            int16_t p_tile_y,
                            AlphaTileId p_alpha_tile_id,
                            uint8_t p_ctrl,
                            int8_t p_backdrop,
                            uint32_t p_path_id,
                            uint32_t p_color)
                : tile_x(p_tile_x), tile_y(p_tile_y),
                  alpha_tile_id(p_alpha_tile_id), ctrl(p_ctrl),
                  backdrop(p_backdrop), path_id(p_path_id), color(p_color) {}
    };
}

#endif //PATHFINDER_D3D9_GPU_DATA_H
