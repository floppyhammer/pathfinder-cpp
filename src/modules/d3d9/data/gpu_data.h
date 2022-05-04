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
        uint32_t metadata_id = 0;
    };
}

#endif //PATHFINDER_D3D9_GPU_DATA_H
