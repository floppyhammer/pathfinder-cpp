//
// Created by floppyhammer on 7/9/2021.
//

#ifndef PATHFINDER_D3D9_DRAW_TILE_BATCH_H
#define PATHFINDER_D3D9_DRAW_TILE_BATCH_H

#include "../../d3d9_d3d11/data/dense_tile_map.h"
#include "gpu_data.h"
#include "../../d3d9_d3d11/effects.h"
#include "../../d3d9_d3d11/paint.h"
#include "../../../rendering/texture.h"
#include "../../../rendering/framebuffer.h"

#include <cstdint>
#include <vector>

namespace Pathfinder {
    struct Clip {
        uint32_t dest_tile_id = 0;
        int32_t dest_backdrop = 0;
        uint32_t src_tile_id = 0;
        int32_t src_backdrop = 0;

        Clip() = default;
    };

    struct DrawTileBatch {
        std::vector<TileObjectPrimitive> tiles;

        // Not in use yet.
        //std::vector<Clip> clips;

        // Tile map size = viewport size / tile size.
        DenseTileMap <uint32_t> z_buffer_data;

        /// The color texture to use.
        RenderTarget color_target;

        /// Render target.
        RenderTarget render_target;

        /// The filter to use.
        //Filter filter;

        /// The blend mode to composite these tiles with.
        BlendMode blend_mode;
    };
}

#endif //PATHFINDER_D3D9_DRAW_TILE_BATCH_H
