#ifndef PATHFINDER_D3D9_ALPHA_TILE_ID_H
#define PATHFINDER_D3D9_ALPHA_TILE_ID_H

#include <cstdint>
#include <cstddef>
#include <limits>

namespace Pathfinder {
    const size_t ALPHA_TILE_LEVEL_COUNT = 2;
    const size_t ALPHA_TILES_PER_LEVEL = 1 << (32 - ALPHA_TILE_LEVEL_COUNT + 1);

    struct AlphaTileId {
        AlphaTileId() = default;

        // Data racing happens here.
        AlphaTileId(size_t *next_alpha_tile_index, int level);

        /// A valid value means a solid tile. Default is invalid.
        uint32_t value = std::numeric_limits<uint32_t>::max();

        bool is_valid() const;
    };
}

#endif //PATHFINDER_D3D9_ALPHA_TILE_ID_H
