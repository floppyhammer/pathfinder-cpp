#ifndef PATHFINDER_D3D11_RENDERER_H
#define PATHFINDER_D3D11_RENDERER_H

#include "../../gpu/buffer.h"
#include "../../gpu/compute_pipeline.h"
#include "../../gpu/descriptor_set.h"
#include "../renderer.h"
#include "../scene.h"
#include "gpu_data.h"
#include "scene_builder.h"

#ifdef PATHFINDER_USE_D3D11

namespace Pathfinder {
// Buffer data
struct TileBatchInfoD3D11 {
    uint32_t tile_count;
    std::shared_ptr<Buffer> z_buffer_id;
    std::shared_ptr<Buffer> tiles_d3d11_buffer_id;
    std::shared_ptr<Buffer> propagate_metadata_buffer_id;
    std::shared_ptr<Buffer> first_tile_map_buffer_id;

    void clean();
};

struct FillBufferInfoD3D11 {
    std::shared_ptr<Buffer> fill_vertex_buffer_id;
};

struct PropagateMetadataBufferIDsD3D11 {
    std::shared_ptr<Buffer> propagate_metadata;
    std::shared_ptr<Buffer> backdrops;
};

struct MicrolinesBufferIDsD3D11 {
    std::shared_ptr<Buffer> buffer_id; // Microlines buffer.
    uint32_t count;                    // Microline count.
};

struct SceneSourceBuffers {
    std::shared_ptr<Buffer> points_buffer; // General buffer.
    uint32_t points_capacity = 0;
    std::shared_ptr<Buffer> point_indices_buffer; // General buffer.
    uint32_t point_indices_count = 0;
    uint32_t point_indices_capacity = 0;

    /// Upload segments to buffer.
    void upload(const std::shared_ptr<Pathfinder::Driver> &driver, SegmentsD3D11 &segments);
};

struct SceneBuffers {
    SceneSourceBuffers draw;
    SceneSourceBuffers clip;

    ~SceneBuffers();

    /// Upload draw and clip segments to buffers.
    void upload(const std::shared_ptr<Pathfinder::Driver> &driver,
                SegmentsD3D11 &draw_segments,
                SegmentsD3D11 &clip_segments);
};

struct PropagateTilesInfoD3D11 {
    Range alpha_tile_range;
};

struct MaskStorage {
    uint64_t framebuffer_id;
    uint32_t allocated_page_count;
};

class RendererD3D11 : public Renderer {
public:
    explicit RendererD3D11(const std::shared_ptr<Pathfinder::Driver> &driver);

    void set_up_pipelines() override;

    void draw(const std::shared_ptr<SceneBuilder> &scene_builder) override;

    std::shared_ptr<Texture> get_dest_texture() override;

    void set_dest_texture(const std::shared_ptr<Texture> &texture) override;

private:
    /// RenderCommand::DrawTilesD3D11(draw_tile_batch)
    void prepare_and_draw_tiles(DrawTileBatchD3D11 &batch, const std::vector<TextureMetadataEntry> &paint_metadata);

    /**
     * Computes backdrops, performs clipping, and populates Z buffers on GPU.
     */
    void prepare_tiles(TileBatchDataD3D11 &batch);

    /**
     * Dice (flatten) segments into microlines. We might have to do this twice if our
     * first attempt runs out of space in the storage buffer.
     * @param dice_metadata
     * @param batch_segment_count
     * @param path_source
     * @param transform
     * @note COMPUTE INPUT dice_metadata_buffer
     * @note COMPUTE OUTPUT microlines_buffer
     */
    MicrolinesBufferIDsD3D11 dice_segments(std::vector<DiceMetadataD3D11> &dice_metadata,
                                           uint32_t batch_segment_count,
                                           PathSource path_source,
                                           Transform2 transform);

    /**
     * Initializes the tile maps.
     * @param tiles_d3d11_buffer_id
     * @param tile_count
     * @param tile_path_info
     * @BufferWrite Tiles buffer
     */
    void bound(const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
               uint32_t tile_count,
               std::vector<TilePathInfoD3D11> &tile_path_info);

    /**
     * Dice (flatten) segments into micro-lines. We might have to do this twice if our
     * first attempt runs out of space in the storage buffer.
     * @BufferRead Microlines buffer, propagate metadata buffers, tiles buffer, z buffer
     * @BufferWrite Fill vertex buffer
     */
    FillBufferInfoD3D11 bin_segments(MicrolinesBufferIDsD3D11 &microlines_storage,
                                     PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids,
                                     const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
                                     const std::shared_ptr<Buffer> &z_buffer_id);

    PropagateTilesInfoD3D11 propagate_tiles(uint32_t column_count,
                                            const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
                                            const std::shared_ptr<Buffer> &z_buffer_id,
                                            const std::shared_ptr<Buffer> &first_tile_map_buffer_id,
                                            const std::shared_ptr<Buffer> &alpha_tiles_buffer_id,
                                            PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids);

    void draw_fills(FillBufferInfoD3D11 &fill_storage_info,
                    const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
                    const std::shared_ptr<Buffer> &alpha_tiles_buffer_id,
                    PropagateTilesInfoD3D11 &propagate_tiles_info);

    /**
     * Called by prepare_tiles().
     */
    void sort_tiles(const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
                    const std::shared_ptr<Buffer> &first_tile_map_buffer_id,
                    const std::shared_ptr<Buffer> &z_buffer_id);

    void draw_tiles(const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
                    const std::shared_ptr<Buffer> &first_tile_map_buffer_id,
                    const RenderTarget &render_target,
                    const std::shared_ptr<Texture> &color_texture);

    void upload_initial_backdrops(const std::shared_ptr<Buffer> &backdrops_buffer_id,
                                  std::vector<BackdropInfoD3D11> &backdrops);

    std::shared_ptr<Buffer> allocate_z_buffer();

    std::shared_ptr<Buffer> allocate_first_tile_map();

    std::shared_ptr<Buffer> allocate_alpha_tile_info(uint32_t index_count);

    /// RenderCommand::UploadSceneD3D11
    /// Upload scene to GPU.
    void upload_scene(SegmentsD3D11 &draw_segments, SegmentsD3D11 &clip_segments);

    PropagateMetadataBufferIDsD3D11 upload_propagate_metadata(std::vector<PropagateMetadataD3D11> &propagate_metadata,
                                                              std::vector<BackdropInfoD3D11> &backdrops);

    Vec2<uint32_t> tile_size() const;

    Vec2<uint32_t> framebuffer_tile_size();

    // Unlike D3D9, we only need mask/dest textures instead of mask/dest framebuffers.
    std::shared_ptr<Texture> mask_texture, dest_texture;

    std::shared_ptr<ComputePipeline> bound_pipeline, dice_pipeline, bin_pipeline, propagate_pipeline, sort_pipeline,
        fill_pipeline, tile_pipeline;

    /// Uniform buffers.
    std::shared_ptr<Buffer> bin_ub, bound_ub, dice_ub0, dice_ub1, fill_ub, propagate_ub, sort_ub, tile_ub0, tile_ub1;

    std::shared_ptr<DescriptorSet> bound_descriptor_set, dice_descriptor_set, bin_descriptor_set,
        propagate_descriptor_set, sort_descriptor_set, fill_descriptor_set, tile_descriptor_set;

    uint32_t allocated_microline_count = 0;
    uint32_t allocated_fill_count = 0;

    SceneBuffers scene_buffers;

    uint32_t alpha_tile_count = 0;

    std::vector<TileBatchInfoD3D11> tile_batch_info;
};
} // namespace Pathfinder

#endif

#endif // PATHFINDER_D3D11_RENDERER_H
