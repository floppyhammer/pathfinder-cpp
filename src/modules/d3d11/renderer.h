//
// Created by floppyhammer on 8/26/2021.
//

#ifndef PATHFINDER_D3D11_RENDERER_H
#define PATHFINDER_D3D11_RENDERER_H

#include "gpu_data.h"
#include "scene_builder.h"
#include "../d3d9_d3d11/scene.h"
#include "../d3d9/data/draw_tile_batch.h"
#include "../../rendering/compute_program.h"
#include "../d3d9_d3d11/renderer.h"

#ifdef PATHFINDER_USE_D3D11

namespace Pathfinder {
    // Buffer data
    struct TileBatchInfoD3D11 {
        uint32_t tile_count;
        uint64_t z_buffer_id;
        uint64_t tiles_d3d11_buffer_id;
        uint64_t propagate_metadata_buffer_id;
        uint64_t first_tile_map_buffer_id;

        void clean();
    };

    struct FillBufferInfoD3D11 {
        uint64_t fill_vertex_buffer_id;
    };

    struct PropagateMetadataBufferIDsD3D11 {
        uint64_t propagate_metadata;
        uint64_t backdrops;
    };

    struct MicrolinesBufferIDsD3D11 {
        uint64_t buffer_id; // Microlines buffer ID.
        uint32_t count; // Microline count.
    };

    struct SceneSourceBuffers {
        uint64_t points_buffer = 0; // General buffer ID.
        uint32_t points_capacity = 0;
        uint64_t point_indices_buffer = 0; // General buffer ID.
        uint32_t point_indices_count = 0;
        uint32_t point_indices_capacity = 0;

        /// Upload segments to buffer.
        void upload(SegmentsD3D11 &segments);
    };

    struct SceneBuffers {
        SceneSourceBuffers draw;
        SceneSourceBuffers clip;

        ~SceneBuffers();

        /// Upload draw and clip segments to buffers.
        void upload(SegmentsD3D11 &draw_segments,
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
        explicit RendererD3D11(const Vec2<int> &p_viewport_size);

        void draw(SceneBuilderD3D11 &scene_builder);

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
        void bound(uint64_t tiles_d3d11_buffer_id,
                   uint32_t tile_count,
                   std::vector<TilePathInfoD3D11> &tile_path_info);

        /**
         * Dice (flatten) segments into micro-lines. We might have to do this twice if our
         * first attempt runs out of space in the storage buffer.
         * @BufferRead Microlines buffer, propagate metadata buffers, tiles buffer, z buffer
         * @BufferWrite Fill vertex buffer
         */
        FillBufferInfoD3D11 bin_segments(
                MicrolinesBufferIDsD3D11 &microlines_storage,
                PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids,
                uint64_t tiles_d3d11_buffer_id,
                uint64_t z_buffer_id);

        PropagateTilesInfoD3D11 propagate_tiles(uint32_t column_count,
                                                uint64_t tiles_d3d11_buffer_id,
                                                uint64_t z_buffer_id,
                                                uint64_t first_tile_map_buffer_id,
                                                uint64_t alpha_tiles_buffer_id,
                                                PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids);

        void draw_fills(FillBufferInfoD3D11 &fill_storage_info,
                        uint64_t tiles_d3d11_buffer_id,
                        uint64_t alpha_tiles_buffer_id,
                        PropagateTilesInfoD3D11 &propagate_tiles_info);

        /**
         * Called by prepare_tiles().
         */
        void sort_tiles(uint64_t tiles_d3d11_buffer_id,
                        uint64_t first_tile_map_buffer_id,
                        uint64_t z_buffer_id);

        void draw_tiles(uint64_t tiles_d3d11_buffer_id,
                        uint64_t first_tile_map_buffer_id,
                        const RenderTarget &target_viewport,
                        const RenderTarget &color_texture);

        void upload_initial_backdrops(uint64_t backdrops_buffer_id,
                                      std::vector<BackdropInfoD3D11> &backdrops);

        uint64_t allocate_z_buffer();

        uint64_t allocate_first_tile_map();

        uint64_t allocate_alpha_tile_info(uint32_t index_count);

        /// RenderCommand::UploadSceneD3D11
        /// Upload scene to GPU.
        void upload_scene(SegmentsD3D11 &draw_segments, SegmentsD3D11 &clip_segments);

        PropagateMetadataBufferIDsD3D11 upload_propagate_metadata(
                std::vector<PropagateMetadataD3D11> &propagate_metadata,
                std::vector<BackdropInfoD3D11> &backdrops);

        Vec2<int> tile_size() const;

        Vec2<int> framebuffer_tile_size();

        // Unlike D3D9, we only need a mask texture instead of a mask viewport.
        std::shared_ptr<Texture> mask_texture;

        std::shared_ptr<ComputeProgram> bound_program;
        std::shared_ptr<ComputeProgram> dice_program;
        std::shared_ptr<ComputeProgram> bin_program;
        std::shared_ptr<ComputeProgram> propagate_program;
        std::shared_ptr<ComputeProgram> sort_program;
        std::shared_ptr<ComputeProgram> fill_program;
        std::shared_ptr<ComputeProgram> tile_program;

        uint32_t allocated_microline_count = 0;
        uint32_t allocated_fill_count = 0;

        SceneBuffers scene_buffers;

        uint32_t alpha_tile_count = 0;

        std::vector<TileBatchInfoD3D11> tile_batch_info;
    };
}

#endif

#endif //PATHFINDER_D3D11_RENDERER_H
