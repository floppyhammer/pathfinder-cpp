#ifndef PATHFINDER_D3D11_RENDERER_H
#define PATHFINDER_D3D11_RENDERER_H

#include "../../gpu/compute_pipeline.h"
#include "../../gpu/descriptor_set.h"
#include "../renderer.h"
#include "../scene.h"
#include "gpu_data.h"

#ifdef PATHFINDER_ENABLE_D3D11

namespace Pathfinder {

struct TileBatchInfoD3D11 {
    uint32_t tile_count;

    uint64_t z_buffer_id;
    uint64_t tiles_d3d11_buffer_id;
    uint64_t propagate_metadata_buffer_id;
    uint64_t first_tile_map_buffer_id;
};

struct FillBufferInfoD3D11 {
    uint64_t fill_vertex_buffer_id;
};

struct PropagateMetadataBufferIDsD3D11 {
    uint64_t propagate_metadata;
    uint64_t backdrops;
};

struct MicrolinesBufferIDsD3D11 {
    uint64_t buffer_id;
    uint32_t count;
};

struct SceneSourceBuffers {
    std::shared_ptr<uint64_t> points_buffer;
    uint32_t points_capacity = 0;

    std::shared_ptr<uint64_t> point_indices_buffer;
    uint32_t point_indices_count = 0;
    uint32_t point_indices_capacity = 0;

    /// Upload segments to buffers.
    void upload(SegmentsD3D11 &segments,
                const std::shared_ptr<GpuMemoryAllocator> &allocator,
                const std::shared_ptr<Device> &device,
                const std::shared_ptr<CommandEncoder> &encoder);
};

struct SceneBuffers {
    SceneSourceBuffers draw;
    SceneSourceBuffers clip;

    /// Upload draw and clip segments to buffers.
    void upload(SegmentsD3D11 &draw_segments,
                SegmentsD3D11 &clip_segments,
                const std::shared_ptr<GpuMemoryAllocator> &allocator,
                const std::shared_ptr<Device> &device,
                const std::shared_ptr<CommandEncoder> &encoder);
};

struct PropagateTilesInfoD3D11 {
    Range alpha_tile_range;
};

struct ClipBufferIDs {
    /// Optional
    uint64_t metadata;

    uint64_t tiles;
};

struct TileUniformD3d11 {
    float clear_color[4]{};
    int32_t load_action{}, pad0, pad1, pad2;
    Vec2F tile_size;
    Vec2F texture_metadata_size;
    Vec2F framebuffer_size;
    Vec2I framebuffer_tile_size;
    Vec2F mask_texture_size;
    Vec2F color_texture_size;
};

class RendererD3D11 : public Renderer {
public:
    explicit RendererD3D11(const std::shared_ptr<Device> &device, const std::shared_ptr<Queue> &queue);

    void set_up_pipelines() override;

    void draw(const std::shared_ptr<SceneBuilder> &scene_builder, bool _clear_dst_texture) override;

    std::shared_ptr<Texture> get_dest_texture() override;

    void set_dest_texture(const std::shared_ptr<Texture> &new_texture) override;

private:
    /// RenderCommand::DrawTilesD3D11(draw_tile_batch)
    void prepare_and_draw_tiles(DrawTileBatchD3D11 &batch);

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
    std::shared_ptr<MicrolinesBufferIDsD3D11> dice_segments(std::vector<DiceMetadataD3D11> &dice_metadata,
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
    void bound(uint64_t tiles_d3d11_buffer_id, uint32_t tile_count, std::vector<TilePathInfoD3D11> &tile_path_info);

    /**
     * Dice (flatten) segments into micro-lines. We might have to do this twice if our
     * first attempt runs out of space in the storage buffer.
     * @BufferRead Microlines buffer, propagate metadata buffers, tiles buffer, z buffer
     * @BufferWrite Fill vertex buffer
     */
    std::shared_ptr<FillBufferInfoD3D11> bin_segments(MicrolinesBufferIDsD3D11 &microlines_storage,
                                                      PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids,
                                                      uint64_t tiles_d3d11_buffer_id,
                                                      uint64_t z_buffer_id);

    PropagateTilesInfoD3D11 propagate_tiles(uint32_t column_count,
                                            uint64_t tiles_d3d11_buffer_id,
                                            uint64_t z_buffer_id,
                                            uint64_t first_tile_map_buffer_id,
                                            uint64_t alpha_tiles_buffer_id,
                                            PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids,
                                            const std::shared_ptr<const ClipBufferIDs> &clip_buffer_ids);

    void draw_fills(FillBufferInfoD3D11 &fill_storage_info,
                    uint64_t tiles_d3d11_buffer_id,
                    uint64_t alpha_tiles_buffer_id,
                    PropagateTilesInfoD3D11 &propagate_tiles_info);

    /**
     * Called by prepare_tiles().
     */
    void sort_tiles(uint64_t tiles_d3d11_buffer_id, uint64_t first_tile_map_buffer_id, uint64_t z_buffer_id);

    void draw_tiles(uint64_t tiles_d3d11_buffer_id,
                    uint64_t first_tile_map_buffer_id,
                    const std::shared_ptr<const RenderTargetId> &render_target_id,
                    const std::shared_ptr<const TileBatchTextureInfo> &color_texture_info);

    void upload_initial_backdrops(uint64_t backdrops_buffer_id, std::vector<BackdropInfoD3D11> &backdrops);

    uint64_t allocate_z_buffer();

    uint64_t allocate_first_tile_map();

    uint64_t allocate_alpha_tile_info(uint32_t index_count);

    /// RenderCommand::UploadSceneD3D11
    /// Upload scene to GPU.
    void upload_scene(SegmentsD3D11 &draw_segments, SegmentsD3D11 &clip_segments);

    PropagateMetadataBufferIDsD3D11 upload_propagate_metadata(std::vector<PropagateMetadataD3D11> &propagate_metadata,
                                                              std::vector<BackdropInfoD3D11> &backdrops);

    Vec2I tile_size() const;

    Vec2I framebuffer_tile_size();

    void free_tile_batch_buffers();

    TextureFormat mask_texture_format() const override;

    void reallocate_alpha_tile_pages_if_necessary();

private:
    // Unlike D3D9, we only need mask/dest textures instead of mask/dest framebuffers.
    std::shared_ptr<Texture> dest_texture;

    std::shared_ptr<ComputePipeline> bound_pipeline, dice_pipeline, bin_pipeline, propagate_pipeline, sort_pipeline,
        fill_pipeline, tile_pipeline;

    /// Uniform buffers.
    uint64_t bin_ub_id, bound_ub_id, dice_ub0_id, dice_ub1_id, fill_ub_id, propagate_ub_id, sort_ub_id, tile_ub_id;

    std::shared_ptr<DescriptorSet> bound_descriptor_set, dice_descriptor_set, bin_descriptor_set,
        propagate_descriptor_set, sort_descriptor_set, fill_descriptor_set, tile_descriptor_set;

    uint32_t allocated_microline_count = 0;
    uint32_t allocated_fill_count = 0;

    SceneBuffers scene_buffers;

    std::vector<TileBatchInfoD3D11> tile_batch_info;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_D3D11_RENDERER_H
