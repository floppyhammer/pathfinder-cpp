//
// Created by floppyhammer on 8/26/2021.
//

#ifndef PATHFINDER_D3D11_GPU_DATA_H
#define PATHFINDER_D3D11_GPU_DATA_H

#include "../d3d9/data/line_segment.h"
#include "../../common/math/transform2.h"
#include "../../common/math/rect.h"
#include "../d3dx/data/built_path.h"
#include "../d3dx/paint.h"
#include "../d3dx/scene.h"

#include <vector>

#ifdef PATHFINDER_USE_D3D11

namespace Pathfinder {
    struct GlobalPathId {
        uint32_t batch_id;
        uint32_t path_index;
    };

    struct FirstTileD3D11 {
        int32_t first_tile = -1;
    };

    struct MicrolineD3D11 {
        int16_t from_x_px;
        int16_t from_y_px;
        int16_t to_x_px;
        int16_t to_y_px;
        uint8_t from_x_subpx;
        uint8_t from_y_subpx;
        uint8_t to_x_subpx;
        uint8_t to_y_subpx;
        uint32_t path_index;
    };

    struct TileD3D11 {
        int32_t next_tile_id;
        int32_t first_fill_id;
        int16_t alpha_tile_id_lo;
        int8_t alpha_tile_id_hi;
        int8_t backdrop_delta;
        uint16_t color;
        uint8_t ctrl;
        int8_t backdrop;
    };

    struct AlphaTileD3D11 {
        uint32_t alpha_tile_index;
        uint32_t clip_tile_index;
    };

    struct BackdropInfoD3D11 {
        int32_t initial_backdrop;

        /// Column number, where 0 is the leftmost column in the tile rect.
        int32_t tile_x_offset;

        /// Together with the `TileBatchId`, uniquely identifies a path on the renderer side.
        uint32_t path_index;

        BackdropInfoD3D11(int32_t p_initial_backdrop, int32_t p_tile_x_offset, uint32_t p_path_index)
                : initial_backdrop(p_initial_backdrop), tile_x_offset(p_tile_x_offset), path_index(p_path_index) {}
    };

    struct PropagateMetadataD3D11 {
        Rect<int32_t> tile_rect;
        uint32_t tile_offset = 0;
        uint32_t path_index = 0;
        uint32_t z_write = 0;

        // This will generally not refer to the same batch as `path_index`.
        uint32_t clip_path_index = 0;
        uint32_t backdrop_offset = 0;
        uint32_t pad0 = 0;
        uint32_t pad1 = 0;
        uint32_t pad2 = 0;

        PropagateMetadataD3D11(const Rect<int32_t>& p_tile_rect,
                               uint32_t p_tile_offset,
                               uint32_t p_path_index,
                               uint32_t p_z_write,
                               uint32_t p_clip_path_index,
                               uint32_t p_backdrop_offset)
                : tile_rect(p_tile_rect),
                  tile_offset(p_tile_offset),
                  path_index(p_path_index),
                  z_write(p_z_write),
                  clip_path_index(p_clip_path_index),
                  backdrop_offset(p_backdrop_offset) {}
    };

    struct DiceMetadataD3D11 {
        /// Either a draw path ID or a clip path ID, depending on context.
        uint32_t global_path_id = 0;
        uint32_t first_global_segment_index = 0;
        uint32_t first_batch_segment_index = 0;
        uint32_t pad = 0;

        DiceMetadataD3D11(uint32_t p_global_path_id,
                          uint32_t p_first_global_segment_index,
                          uint32_t p_first_batch_segment_index)
                : global_path_id(p_global_path_id),
                  first_global_segment_index(p_first_global_segment_index),
                  first_batch_segment_index(p_first_batch_segment_index) {}
    };

    struct TilePathInfoD3D11 {
        int16_t tile_min_x = 0;
        int16_t tile_min_y = 0;
        int16_t tile_max_x = 0;
        int16_t tile_max_y = 0;
        uint32_t first_tile_index = 0;
        // Must match the order in `TileD3D11`.
        uint16_t color = 0;
        uint8_t ctrl = 0;
        int8_t backdrop = 0;

        TilePathInfoD3D11(int16_t p_tile_min_x,
                          int16_t p_tile_min_y,
                          int16_t p_tile_max_x,
                          int16_t p_tile_max_y,
                          uint32_t p_first_tile_index,
                          uint16_t p_color,
                          uint8_t p_ctrl,
                          int8_t p_backdrop)
                : tile_min_x(p_tile_min_x),
                  tile_min_y(p_tile_min_y),
                  tile_max_x(p_tile_max_x),
                  tile_max_y(p_tile_max_y),
                  first_tile_index(p_first_tile_index),
                  color(p_color),
                  ctrl(p_ctrl),
                  backdrop(p_backdrop) {}
    };

    /// Information about a batch of tiles to be prepared on GPU.
    struct PrepareTilesInfoD3D11 {
        /// Initial backdrop values for each tile column, packed together.
        std::vector<BackdropInfoD3D11> backdrops;

        /// Mapping from path index to metadata needed to compute propagation on GPU.
        /// This contains indices into the `tiles` vector.
        std::vector<PropagateMetadataD3D11> propagate_metadata;

        /// Metadata about each path that will be diced (flattened).
        std::vector<DiceMetadataD3D11> dice_metadata;

        /// Sparse information about all the allocated tiles.
        std::vector<TilePathInfoD3D11> tile_path_info;

        /// A transform to apply to the segments.
        Transform2 transform;
    };

    /// Where a path should come from (draw or clip).
    enum PathSource {
        Draw,
        Clip,
    };

    /// Information about a batch of tiles to be prepared (post-processed).
    struct TileBatchDataD3D11 {
        TileBatchDataD3D11() = default;

        TileBatchDataD3D11(uint32_t p_batch_id,
                           PathSource p_path_source);

        uint32_t push(BuiltPath &path,
                      uint32_t global_path_id,
                      bool z_write,
                      LastSceneInfo &last_scene);

        /// The ID of this batch.
        /// The renderer should not assume that these values are consecutive.
        uint32_t batch_id = 0;

        /// The number of paths in this batch.
        uint32_t path_count = 0;

        /// The number of tiles in this batch.
        uint32_t tile_count = 0;

        /// The total number of segments in this batch.
        uint32_t segment_count = 0;

        /// Information needed to prepare the tiles.
        PrepareTilesInfoD3D11 prepare_info;

        /// Where the paths come from (draw or clip).
        PathSource path_source = PathSource::Draw;
    };

    struct TileBatchTexture {
        uint32_t page = 0;
        TextureSamplingFlags sampling_flags;
        PaintCompositeOp composite_op{};
    };

    /// Information needed to draw a batch of tiles in D3D11.
    struct DrawTileBatchD3D11 {
        /// Data for the tile batch.
        TileBatchDataD3D11 tile_batch_data;

        /// The color texture to use.
        //TileBatchTexture color_texture;

        /// The color texture to use.
        RenderTarget color_texture;

        /// Render target.
        RenderTarget viewport;
    };

    struct SegmentIndicesD3D11 {
        // First point in the segment.
        uint32_t first_point_index = 0;

        // Segment type, i.e. line, quadratic, or cubic.
        uint32_t flag = 0;

        SegmentIndicesD3D11(uint32_t p_first_point_index, uint32_t p_flag)
                : first_point_index(p_first_point_index), flag(p_flag) {}
    };

    struct SegmentsD3D11 {
        std::vector<Vec2<float>> points;
        std::vector<SegmentIndicesD3D11> indices;

        /// Add a shape as segments.
        Range add_path(const Shape& shape);
    };
}

#endif

#endif //PATHFINDER_D3D11_GPU_DATA_H
