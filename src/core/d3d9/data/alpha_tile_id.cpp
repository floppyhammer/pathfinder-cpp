#include "alpha_tile_id.h"

namespace Pathfinder {
    AlphaTileId::AlphaTileId(std::array<std::atomic<size_t>, ALPHA_TILE_LEVEL_COUNT> &next_alpha_tile_index, int level) {
        // Atomic fetch & add.
        size_t alpha_tile_index = next_alpha_tile_index[level].fetch_add(1);

        value = level * ALPHA_TILES_PER_LEVEL + alpha_tile_index;
    }

    bool AlphaTileId::is_valid() const {
        return value < std::numeric_limits<uint32_t>::max();
    }
}
