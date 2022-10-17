#ifndef PATHFINDER_D3D9_DRAW_TILE_BATCH_H
#define PATHFINDER_D3D9_DRAW_TILE_BATCH_H

#include <cstdint>
#include <vector>

#include "../../../gpu/framebuffer.h"
#include "../../../gpu/texture.h"
#include "../../data/data.h"
#include "../../data/dense_tile_map.h"
#include "../../paint/effects.h"
#include "../../paint/paint.h"
#include "gpu_data.h"

namespace Pathfinder {

struct DrawTileBatch {
    std::vector<TileObjectPrimitive> tiles;

//    std::vector<Clip> clips;

    // Tile map size = viewport size / tile size.
    DenseTileMap<uint32_t> z_buffer_data;

    /// The color texture to use.
    std::shared_ptr<Texture> color_texture;

    std::shared_ptr<Texture> metadata_texture;

    /// Render target.
    RenderTarget render_target;

    /// The filter to use.
    // Filter filter;

    /// The blend mode to composite these tiles with.
    BlendMode blend_mode;
};

} // namespace Pathfinder

#endif // PATHFINDER_D3D9_DRAW_TILE_BATCH_H
