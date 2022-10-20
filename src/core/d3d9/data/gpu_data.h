#ifndef PATHFINDER_D3D9_GPU_DATA_H
#define PATHFINDER_D3D9_GPU_DATA_H

#include "../../data/line_segment.h"
#include "alpha_tile_id.h"

namespace Pathfinder {

//! Data that will be sent to GPU directly.

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
    AlphaTileId alpha_tile_id;
    uint8_t ctrl = 0; // Fill rule
    int8_t backdrop = 0;
    uint32_t path_id = 0;
    uint32_t metadata_id = 0;
};

/// A vector of this will be sent the tile clip copy program.
struct Clip {
    AlphaTileId dest_tile_id;
    int32_t dest_backdrop = 0;
    AlphaTileId src_tile_id;
    int32_t src_backdrop = 0;
};

} // namespace Pathfinder

#endif // PATHFINDER_D3D9_GPU_DATA_H
