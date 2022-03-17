//
// Created by chy on 7/9/2021.
//

#include "alpha_tile_id.h"

namespace Pathfinder {
    AlphaTileId::AlphaTileId(size_t *next_alpha_tile_index, int level) {
        size_t alpha_tile_index;

        // Atomic fetch & add.
#pragma omp critical(update_alpha_tile_index)
        {
            // Fetch.
            alpha_tile_index = next_alpha_tile_index[level];

            // Add.
            next_alpha_tile_index[level] += 1;
        }

        value = level * ALPHA_TILES_PER_LEVEL + alpha_tile_index;
    }

    bool AlphaTileId::is_valid() const {
        return value < std::numeric_limits<uint32_t>::max();
    }
}
