#ifndef PATHFINDER_D3D9_DRAW_TILE_BATCH_H
#define PATHFINDER_D3D9_DRAW_TILE_BATCH_H

#include <cstdint>
#include <vector>

#include "../../data/data.h"
#include "../../data/dense_tile_map.h"
#include "../../paint/effects.h"
#include "../../paint/paint.h"
#include "gpu_data.h"

namespace Pathfinder {

/// Tiles in a batch use the same color texture and render target.
struct DrawTileBatchD3D9 {
    std::vector<TileObjectPrimitive> tiles;

    std::vector<Clip> clips;

    // Tile map size = viewport size / tile size.
    DenseTileMap<uint32_t> z_buffer_data;

    /// The color texture to use.
    std::shared_ptr<TileBatchTextureInfo> color_texture_info;

    /// Render target ID.
    std::shared_ptr<RenderTargetId> render_target_id;

    /// The filter to use.
    // Filter filter;

    /// The blend mode to composite these tiles with.
    BlendMode blend_mode;
};

} // namespace Pathfinder

#endif // PATHFINDER_D3D9_DRAW_TILE_BATCH_H
