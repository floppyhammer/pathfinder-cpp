#ifndef PATHFINDER_D3D11_GPU_DATA_H
#define PATHFINDER_D3D11_GPU_DATA_H

#include <vector>

#include "../../common/math/rect.h"
#include "../../common/math/transform2.h"
#include "../data/built_path.h"
#include "../data/line_segment.h"
#include "../paint/paint.h"
#include "../scene.h"

#ifdef PATHFINDER_ENABLE_D3D11

//! Data that will be sent directly to GPU.

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
};

struct PropagateMetadataD3D11 {
    RectI tile_rect;
    uint32_t tile_offset = 0;
    uint32_t path_index = 0;
    uint32_t z_write = 0;

    // This will generally not refer to the same batch as `path_index`.
    uint32_t clip_path_index = 0;
    uint32_t backdrop_offset = 0;
    uint32_t pad0 = 0;
    uint32_t pad1 = 0;
    uint32_t pad2 = 0;
};

struct DiceMetadataD3D11 {
    /// Either a draw path ID or a clip path ID, depending on context.
    uint32_t global_path_id = 0;
    uint32_t first_global_segment_index = 0;
    uint32_t first_batch_segment_index = 0;
    uint32_t pad = 0;
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
};

/// Information about a batch of tiles to be prepared on GPU.
struct PrepareTilesInfoD3D11 {
    /// Initial backdrop values for each tile column, packed together.
    std::vector<BackdropInfoD3D11> backdrops;

    /// Mapping from path index to metadata needed to compute propagation on GPU.
    /// This contains indices into the `tiles` std::vector.
    std::vector<PropagateMetadataD3D11> propagate_metadata;

    /// Metadata about each path that will be diced (flattened).
    std::vector<DiceMetadataD3D11> dice_metadata;

    /// Sparse information about all the allocated tiles.
    std::vector<TilePathInfoD3D11> tile_path_info;

    /// A transform to apply to the segments.
    Transform2 transform;
};

/// Where a path should come from (draw or clip).
enum class PathSource {
    Draw,
    Clip,
};

/// Information about clips applied to paths in a batch.
struct ClippedPathInfo {
    /// The ID of the batch containing the clips.
    uint32_t clip_batch_id;

    /// The number of paths that have clips.
    uint32_t clipped_path_count;

    /// The maximum number of clipped tiles.
    ///
    /// This is used to allocate vertex buffers.
    uint32_t max_clipped_tile_count;

    /// The actual clips, if calculated on CPU.
    std::shared_ptr<std::vector<Clip>> clips;
};

/// Information about a batch of tiles to be prepared (post-processed).
struct TileBatchDataD3D11 {
    TileBatchDataD3D11() = default;

    TileBatchDataD3D11(uint32_t p_batch_id, PathSource p_path_source);

    uint32_t push(const BuiltPath &path,
                  uint32_t global_path_id,
                  const std::shared_ptr<GlobalPathId> &batch_clip_path_id,
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

    /// Information about clips applied to paths, if any of the paths have clips.
    std::shared_ptr<ClippedPathInfo> clipped_path_info;
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
    std::shared_ptr<TileBatchTextureInfo> color_texture_info;

    /// Where to draw this batch.
    std::shared_ptr<RenderTargetId> render_target_id;
};

struct SegmentIndicesD3D11 {
    // First point in the segment.
    uint32_t first_point_index = 0;

    // Segment type, i.e. line, quadratic, or cubic.
    uint32_t flag = 0;
};

struct SegmentsD3D11 {
    std::vector<Vec2F> points;
    std::vector<SegmentIndicesD3D11> indices;

    /// Add a outline as segments.
    Range add_path(const Outline &outline);
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_D3D11_GPU_DATA_H
