#ifndef PATHFINDER_D3D9_DRAW_TILE_BATCH_H
#define PATHFINDER_D3D9_DRAW_TILE_BATCH_H

#include "gpu_data.h"
#include "../../effects.h"
#include "../../paint.h"
#include "../../data/dense_tile_map.h"
#include "../../../gpu/texture.h"
#include "../../../gpu/framebuffer.h"

#include <cstdint>
#include <vector>

namespace Pathfinder {
    struct Clip {
        uint32_t dest_tile_id = 0;
        int32_t dest_backdrop = 0;
        uint32_t src_tile_id = 0;
        int32_t src_backdrop = 0;
    };

    struct DrawTileBatch {
        std::vector<TileObjectPrimitive> tiles;

        // Not in use yet.
        //std::vector<Clip> clips;

        // Tile map size = viewport size / tile size.
        DenseTileMap <uint32_t> z_buffer_data;

        /// The color texture to use.
        std::shared_ptr<Texture> color_texture;

        /// Render target.
        RenderTarget render_target;

        /// The filter to use.
        //Filter filter;

        /// The blend mode to composite these tiles with.
        BlendMode blend_mode;
    };
}

#endif //PATHFINDER_D3D9_DRAW_TILE_BATCH_H
