//
// Created by floppyhammer on 8/26/2021.
//

#include "renderer.h"

#include "gpu_data.h"
#include "../d3d9_d3d11/data/data.h"
#include "../d3d9/data/draw_tile_batch.h"
#include "../../common/math/basic.h"
#include "../../rendering/device_gl.h"
#include "../../common/logger.h"
#include "../../common/timestamp.h"

#ifdef PATHFINDER_USE_D3D11

namespace Pathfinder {
    const size_t FILL_INDIRECT_DRAW_PARAMS_INSTANCE_COUNT_INDEX = 1;
    const size_t FILL_INDIRECT_DRAW_PARAMS_ALPHA_TILE_COUNT_INDEX = 4;
    const size_t FILL_INDIRECT_DRAW_PARAMS_SIZE = 8;

    const size_t BIN_INDIRECT_DRAW_PARAMS_MICROLINE_COUNT_INDEX = 3;

    const uint32_t BOUND_WORKGROUP_SIZE = 64;
    const uint32_t DICE_WORKGROUP_SIZE = 64;
    const uint32_t BIN_WORKGROUP_SIZE = 64;
    const uint32_t PROPAGATE_WORKGROUP_SIZE = 64;
    const uint32_t SORT_WORKGROUP_SIZE = 64;

    // If clear destination texture before drawing using D3D11.
    const int32_t LOAD_ACTION_CLEAR = 0;
    const int32_t LOAD_ACTION_LOAD = 1;

    const uint32_t INITIAL_ALLOCATED_MICROLINE_COUNT = 1024 * 16;
    const uint32_t INITIAL_ALLOCATED_FILL_COUNT = 1024 * 16;

    Vec2<int> pixel_size_to_tile_size(Vec2<int> &pixel_size) {
        // Round up.
        auto tile_size = Vec2<int>(TILE_WIDTH - 1, TILE_HEIGHT - 1);
        auto size = pixel_size + tile_size;
        return {size.x / TILE_WIDTH, size.y / TILE_HEIGHT};
    }

    void TileBatchInfoD3D11::clean() {
        DeviceGl::free_general_buffer(z_buffer_id);
        DeviceGl::free_general_buffer(tiles_d3d11_buffer_id);
        DeviceGl::free_general_buffer(propagate_metadata_buffer_id);
        DeviceGl::free_general_buffer(first_tile_map_buffer_id);
        z_buffer_id = 0;
        tiles_d3d11_buffer_id = 0;
        propagate_metadata_buffer_id = 0;
        first_tile_map_buffer_id = 0;
    }

    void SceneSourceBuffers::upload(SegmentsD3D11 &segments) {
        auto needed_points_capacity = upper_power_of_two(segments.points.size());
        auto needed_point_indices_capacity = upper_power_of_two(segments.indices.size());

        // Reallocate if capacity is not enough.
        if (points_capacity < needed_points_capacity) {
            DeviceGl::free_general_buffer(points_buffer);

            points_buffer = DeviceGl::allocate_general_buffer<Vec2<float>>(needed_points_capacity);

            points_capacity = needed_points_capacity;
        }

        // Reallocate if capacity is not enough.
        if (point_indices_capacity < needed_point_indices_capacity) {
            DeviceGl::free_general_buffer(point_indices_buffer);

            point_indices_buffer = DeviceGl::allocate_general_buffer<SegmentIndicesD3D11>(
                    needed_point_indices_capacity);

            point_indices_capacity = needed_point_indices_capacity;
        }

        DeviceGl::upload_to_general_buffer<Vec2<float>>(
                points_buffer,
                0,
                segments.points.data(),
                segments.points.size());

        DeviceGl::upload_to_general_buffer<SegmentIndicesD3D11>(
                point_indices_buffer,
                0,
                segments.indices.data(),
                segments.indices.size());

        point_indices_count = segments.indices.size();
    }

    SceneBuffers::~SceneBuffers() {
        DeviceGl::free_general_buffer(draw.points_buffer);
        DeviceGl::free_general_buffer(draw.point_indices_buffer);
    }

    void SceneBuffers::upload(SegmentsD3D11 &draw_segments, SegmentsD3D11 &clip_segments) {
        draw.upload(draw_segments);
        //clip.upload(clip_segments);
    }

    RendererD3D11::RendererD3D11(const Vec2<int> &p_viewport_size) : Renderer(p_viewport_size) {
#ifdef PATHFINDER_SHIP_SHADERS
        const std::string dice_source =
#include "../src/shaders/minified/minified_dice.comp"
        ;
        const std::string bound_source =
#include "../src/shaders/minified/minified_bound.comp"
        ;
        const std::string bin_source =
#include "../src/shaders/minified/minified_bin.comp"
        ;
        const std::string propagate_source =
#include "../src/shaders/minified/minified_propagate.comp"
        ;
        const std::string sort_source =
#include "../src/shaders/minified/minified_sort.comp"
        ;
        const std::string fill_source =
#include "../src/shaders/minified/minified_fill.comp"
        ;
        const std::string tile_source_0 =
#include "../src/shaders/minified/minified_tile.comp.0"
        ;
        const std::string tile_source_1 =
#include "../src/shaders/minified/minified_tile.comp.1"
        ;

        bound_program = std::make_shared<ComputeProgram>(bound_source);
        dice_program = std::make_shared<ComputeProgram>(dice_source);
        bin_program = std::make_shared<ComputeProgram>(bin_source);
        propagate_program = std::make_shared<ComputeProgram>(propagate_source);
        sort_program = std::make_shared<ComputeProgram>(sort_source);
        fill_program = std::make_shared<ComputeProgram>(fill_source);
        tile_program = std::make_shared<ComputeProgram>(tile_source_0 + tile_source_1);
#else
        bound_program = std::make_shared<ComputeProgram>(PATHFINDER_SHADER_DIR"d3d11/bound.comp");
        dice_program = std::make_shared<ComputeProgram>(PATHFINDER_SHADER_DIR"d3d11/dice.comp");
        bin_program = std::make_shared<ComputeProgram>(PATHFINDER_SHADER_DIR"d3d11/bin.comp");
        propagate_program = std::make_shared<ComputeProgram>(PATHFINDER_SHADER_DIR"d3d11/propagate.comp");
        sort_program = std::make_shared<ComputeProgram>(PATHFINDER_SHADER_DIR"d3d11/sort.comp");
        fill_program = std::make_shared<ComputeProgram>(PATHFINDER_SHADER_DIR"d3d11/fill.comp");
        tile_program = std::make_shared<ComputeProgram>(PATHFINDER_SHADER_DIR"d3d11/tile.comp");
#endif

        allocated_microline_count = INITIAL_ALLOCATED_MICROLINE_COUNT;
        allocated_fill_count = INITIAL_ALLOCATED_FILL_COUNT;

        // Unlike D3D9, we use RGBA8 here instead of RGBA16F.
        mask_texture = std::make_shared<Texture>(MASK_FRAMEBUFFER_WIDTH,
                                                 MASK_FRAMEBUFFER_HEIGHT,
                                                 TextureFormat::RGBA8,
                                                 DataType::UNSIGNED_BYTE);
    }

    void RendererD3D11::draw(SceneBuilderD3D11 &scene_builder) {
        // RenderCommand::UploadSceneD3D11
        upload_scene(scene_builder.built_segments.draw_segments, scene_builder.built_segments.clip_segments);

        alpha_tile_count = 0;

        for (auto &batch: scene_builder.tile_batches) {
            prepare_and_draw_tiles(batch, scene_builder.metadata);
        }
    }

    void RendererD3D11::upload_scene(SegmentsD3D11 &draw_segments, SegmentsD3D11 &clip_segments) {
        scene_buffers.upload(draw_segments, clip_segments);
    }

    void RendererD3D11::prepare_and_draw_tiles(DrawTileBatchD3D11 &batch,
                                               const std::vector<TextureMetadataEntry> &metadata) {
        Timestamp timestamp;

        auto tile_batch_id = batch.tile_batch_data.batch_id;

        prepare_tiles(batch.tile_batch_data);

        timestamp.record("prepare_tiles");

        auto batch_info = tile_batch_info[tile_batch_id];

        upload_metadata(metadata_texture, metadata);

        draw_tiles(batch_info.tiles_d3d11_buffer_id,
                   batch_info.first_tile_map_buffer_id,
                   batch.viewport,
                   batch.color_texture);

        timestamp.record("draw_tiles");
        timestamp.print();

        // Free general buffers.
        batch_info.clean();
    }

    void RendererD3D11::draw_tiles(uint64_t tiles_d3d11_buffer_id,
                                   uint64_t first_tile_map_buffer_id,
                                   const RenderTarget &target_viewport,
                                   const RenderTarget &color_texture) {
        // The framebuffer mentioned here is different from the target viewport.
        // This doesn't change as long as the destination texture's size doesn't change.
        auto framebuffer_tile_size0 = framebuffer_tile_size();

        // Decide render target.
        Vec2<int> target_viewport_size;
        int target_texture_id = 0;
        int clear_op;
        // If no specific RenderTarget is given, we render to the destination texture.
        if (target_viewport.framebuffer_id == 0) {
            target_viewport_size = viewport_size;
            target_texture_id = dest_viewport->get_texture_id();
            clear_op = LOAD_ACTION_LOAD;
        } else {
            target_viewport_size = target_viewport.size;
            glBindFramebuffer(GL_FRAMEBUFFER, target_viewport.framebuffer_id);
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                      GL_COLOR_ATTACHMENT0,
                                      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                      &target_texture_id);
            clear_op = LOAD_ACTION_CLEAR;
        }

        tile_program->use();

        // Bind textures.
        tile_program->bind_texture(0, "uTextureMetadata", metadata_texture->get_texture_id());
        tile_program->bind_texture(1, "uZBuffer", 0);
        tile_program->bind_texture(2, "uColorTexture0", color_texture.texture_id);
        tile_program->bind_texture(3, "uMaskTexture0", mask_texture->get_texture_id());
        tile_program->bind_texture(4, "uGammaLUT", 0);

        // Bind dest image.
        tile_program->bind_image(0, target_texture_id, GL_READ_WRITE, GL_RGBA8);

        // Set uniforms.
        tile_program->set_int("uLoadAction", clear_op); // If we should clear the dest texture.
        tile_program->set_vec4("uClearColor", 0, 0, 0, 0); // Clear color for the above op.
        tile_program->set_vec2("uTileSize", TILE_WIDTH, TILE_HEIGHT);
        tile_program->set_vec2("uTextureMetadataSize",
                               TEXTURE_METADATA_TEXTURE_WIDTH,
                               TEXTURE_METADATA_TEXTURE_HEIGHT);
        tile_program->set_vec2i("uZBufferSize", 0, 0);
        if (color_texture.texture_id != 0) {
            tile_program->set_vec2("uColorTextureSize0", color_texture.size.x, color_texture.size.y);
        } else {
            tile_program->set_vec2("uColorTextureSize0", 0, 0);
        }
        tile_program->set_vec2("uMaskTextureSize0", mask_texture->get_width(), mask_texture->get_height());
        tile_program->set_vec2("uFramebufferSize", target_viewport_size.x, target_viewport_size.y);
        tile_program->set_vec2i("uFramebufferTileSize", framebuffer_tile_size0.x, framebuffer_tile_size0.y);

        tile_program->bind_general_buffer(0, tiles_d3d11_buffer_id);
        tile_program->bind_general_buffer(1, first_tile_map_buffer_id);

        tile_program->dispatch(framebuffer_tile_size0.x,
                               framebuffer_tile_size0.y);
    }

    Vec2<int> RendererD3D11::tile_size() const {
        auto temp = viewport_size + Vec2<int>(TILE_WIDTH - 1, TILE_HEIGHT - 1);
        return {temp.x / TILE_WIDTH, temp.y / TILE_HEIGHT};
    }

    uint64_t RendererD3D11::allocate_z_buffer() {
        // This includes the fill indirect draw params because some drivers limit the number of
        // SSBOs to 8 (#373).
        // Add FILL_INDIRECT_DRAW_PARAMS_SIZE in case tile size is zero.
        auto size = tile_size().area() + FILL_INDIRECT_DRAW_PARAMS_SIZE;
        auto buffer_id = DeviceGl::allocate_general_buffer<int32_t>(size);

        return buffer_id;
    }

    uint64_t RendererD3D11::allocate_first_tile_map() {
        auto size = tile_size().area();
        auto buffer_id = DeviceGl::allocate_general_buffer<FirstTileD3D11>(size);

        return buffer_id;
    }

    uint64_t RendererD3D11::allocate_alpha_tile_info(uint32_t index_count) {
        auto buffer_id = DeviceGl::allocate_general_buffer<AlphaTileD3D11>(index_count);

        return buffer_id;
    }

    PropagateMetadataBufferIDsD3D11 RendererD3D11::upload_propagate_metadata(
            std::vector<PropagateMetadataD3D11> &propagate_metadata,
            std::vector<BackdropInfoD3D11> &backdrops) {
        auto propagate_metadata_storage_id = DeviceGl::allocate_general_buffer<PropagateMetadataD3D11>(
                propagate_metadata.size());

        DeviceGl::upload_to_general_buffer<PropagateMetadataD3D11>(
                propagate_metadata_storage_id,
                0,
                propagate_metadata.data(),
                propagate_metadata.size());

        auto backdrops_storage_id = DeviceGl::allocate_general_buffer<BackdropInfoD3D11>(
                backdrops.size());

        return {propagate_metadata_storage_id, backdrops_storage_id};
    }

    void RendererD3D11::upload_initial_backdrops(uint64_t backdrops_buffer_id,
                                                 std::vector<BackdropInfoD3D11> &backdrops) {
        DeviceGl::upload_to_general_buffer<BackdropInfoD3D11>(
                backdrops_buffer_id,
                0,
                backdrops.data(),
                backdrops.size());
    }

    void RendererD3D11::prepare_tiles(TileBatchDataD3D11 &batch) {
        // Upload tiles to GPU or allocate them as appropriate.
        auto tiles_d3d11_buffer_id = DeviceGl::allocate_general_buffer<TileD3D11>(batch.tile_count);

        // Allocate a Z-buffer.
        auto z_buffer_id = allocate_z_buffer();

        // Propagate backdrops, bin fills, render fills, and/or perform clipping on GPU if necessary.
        // Allocate space for tile lists.
        auto first_tile_map_buffer_id = allocate_first_tile_map();

        auto propagate_metadata_buffer_ids = upload_propagate_metadata(
                batch.prepare_info.propagate_metadata,
                batch.prepare_info.backdrops);

        // Dice (flatten) segments into micro-lines. We might have to do this twice if our
        // first attempt runs out of space in the storage buffer.
        MicrolinesBufferIDsD3D11 microlines_storage{};
        for (int i = 0; i < 2; i++) {
            microlines_storage = dice_segments(
                    batch.prepare_info.dice_metadata,
                    batch.segment_count,
                    batch.path_source,
                    batch.prepare_info.transform);

            // If the microlines buffer has been allocated successfully.
            if (microlines_storage.buffer_id != 0)
                break;
        }

        // Initialize tiles, bin segments. We might have to do this twice if our first
        // attempt runs out of space in the fill buffer. If this is the case, we also
        // need to re-initialize tiles and re-upload backdrops because they would have
        // been modified during the first attempt.
        FillBufferInfoD3D11 fill_buffer_info{};
        for (int i = 0; i < 2; i++) {
            // Initialize tiles.
            bound(tiles_d3d11_buffer_id,
                  batch.tile_count,
                  batch.prepare_info.tile_path_info);

            // Upload backdrops data.
            upload_initial_backdrops(propagate_metadata_buffer_ids.backdrops,
                                     batch.prepare_info.backdrops);

            fill_buffer_info = bin_segments(
                    microlines_storage,
                    propagate_metadata_buffer_ids,
                    tiles_d3d11_buffer_id,
                    z_buffer_id);

            // If the fill buffer has been allocated successfully.
            if (fill_buffer_info.fill_vertex_buffer_id != 0) {
                break;
            }
        }

        // Free microlines storage as it's not needed anymore.
        DeviceGl::free_general_buffer(microlines_storage.buffer_id);

        auto alpha_tiles_buffer_id = allocate_alpha_tile_info(batch.tile_count);

        auto propagate_tiles_info = propagate_tiles(
                batch.prepare_info.backdrops.size(),
                tiles_d3d11_buffer_id,
                z_buffer_id,
                first_tile_map_buffer_id,
                alpha_tiles_buffer_id,
                propagate_metadata_buffer_ids);

        DeviceGl::free_general_buffer(propagate_metadata_buffer_ids.backdrops);

        draw_fills(fill_buffer_info,
                   tiles_d3d11_buffer_id,
                   alpha_tiles_buffer_id,
                   propagate_tiles_info);

        DeviceGl::free_general_buffer(fill_buffer_info.fill_vertex_buffer_id);
        DeviceGl::free_general_buffer(alpha_tiles_buffer_id);

        // FIXME(pcwalton): This seems like the wrong place to do this...
        sort_tiles(tiles_d3d11_buffer_id, first_tile_map_buffer_id, z_buffer_id);

        tile_batch_info.insert(tile_batch_info.begin() + batch.batch_id,
                               TileBatchInfoD3D11{
                                       batch.tile_count,
                                       z_buffer_id,
                                       tiles_d3d11_buffer_id,
                                       propagate_metadata_buffer_ids.propagate_metadata,
                                       first_tile_map_buffer_id,
                               });
    }

    MicrolinesBufferIDsD3D11 RendererD3D11::dice_segments(
            std::vector<DiceMetadataD3D11> &dice_metadata,
            uint32_t batch_segment_count,
            PathSource path_source,
            Transform2 transform) {
        // Allocate some general buffers.
        auto microlines_buffer_id = DeviceGl::allocate_general_buffer<MicrolineD3D11>(allocated_microline_count);
        auto dice_metadata_buffer_id = DeviceGl::allocate_general_buffer<DiceMetadataD3D11>(dice_metadata.size());
        auto indirect_draw_params_buffer_id = DeviceGl::allocate_general_buffer<uint32_t>(8);

        // Get scene source buffers.
        auto &scene_source_buffers = path_source == PathSource::Draw ? scene_buffers.draw : scene_buffers.clip;
        auto points_buffer_id = scene_source_buffers.points_buffer;
        auto point_indices_buffer_id = scene_source_buffers.point_indices_buffer;
        auto point_indices_count = scene_source_buffers.point_indices_count;

        // Upload dice indirect draw params, which are also used for output.
        uint32_t indirect_compute_params[8] = {0, 0, 0, 0, point_indices_count, 0, 0, 0};
        DeviceGl::upload_to_general_buffer<uint32_t>(
                indirect_draw_params_buffer_id,
                0,
                indirect_compute_params,
                8);

        // Upload dice metadata.
        DeviceGl::upload_to_general_buffer<DiceMetadataD3D11>(
                dice_metadata_buffer_id,
                0,
                dice_metadata.data(),
                dice_metadata.size());

        // Launch dice program.
        // ----------------------------------------------------
        dice_program->use();

        // Set uniforms.
        dice_program->set_mat2("uTransform", transform.matrix);
        dice_program->set_vec2("uTranslation", transform.vector.x, transform.vector.y);
        dice_program->set_int("uPathCount", dice_metadata.size());
        dice_program->set_int("uLastBatchSegmentIndex", batch_segment_count);
        dice_program->set_int("uMaxMicrolineCount", allocated_microline_count);

        // Bind storage buffers.
        dice_program->bind_general_buffer(0, indirect_draw_params_buffer_id);
        dice_program->bind_general_buffer(1, dice_metadata_buffer_id);
        dice_program->bind_general_buffer(2, points_buffer_id);
        dice_program->bind_general_buffer(3, point_indices_buffer_id);
        dice_program->bind_general_buffer(4, microlines_buffer_id);

        dice_program->dispatch((batch_segment_count + DICE_WORKGROUP_SIZE - 1) / DICE_WORKGROUP_SIZE);
        // ----------------------------------------------------

        // Read indirect draw params back to CPU memory.
        DeviceGl::read_general_buffer<uint32_t>(
                indirect_draw_params_buffer_id,
                0,
                indirect_compute_params,
                8);

        // Free some general buffers which are no longer useful.
        DeviceGl::free_general_buffer(dice_metadata_buffer_id);
        DeviceGl::free_general_buffer(indirect_draw_params_buffer_id);

        // Fetch microline count from indirect draw params.
        auto microline_count = indirect_compute_params[BIN_INDIRECT_DRAW_PARAMS_MICROLINE_COUNT_INDEX];

        // Allocate more space if not allocated enough.
        if (microline_count > allocated_microline_count) {
            allocated_microline_count = upper_power_of_two(microline_count);

            // We need a larger buffer, but we should free the old one first.
            DeviceGl::free_general_buffer(microlines_buffer_id);

            return {0, 0};
        }

        return {microlines_buffer_id, microline_count};
    }

    void RendererD3D11::bound(uint64_t tiles_d3d11_buffer_id,
                              uint32_t tile_count,
                              std::vector<TilePathInfoD3D11> &tile_path_info) {
        // This is a staging buffer, which will be freed in the end of this function.
        auto path_info_buffer_id = DeviceGl::allocate_general_buffer<TilePathInfoD3D11>(tile_path_info.size());

        DeviceGl::upload_to_general_buffer<TilePathInfoD3D11>(path_info_buffer_id,
                                                               0,
                                                               tile_path_info.data(),
                                                               tile_path_info.size());

        bound_program->use();

        // Set uniforms.
        bound_program->set_int("uPathCount", tile_path_info.size());
        bound_program->set_int("uTileCount", tile_count);

        // Bind storage buffers.
        bound_program->bind_general_buffer(0, path_info_buffer_id);
        bound_program->bind_general_buffer(1, tiles_d3d11_buffer_id);

        bound_program->dispatch((tile_count + BOUND_WORKGROUP_SIZE - 1) / BOUND_WORKGROUP_SIZE);

        DeviceGl::free_general_buffer(path_info_buffer_id);
    }

    FillBufferInfoD3D11 RendererD3D11::bin_segments(
            MicrolinesBufferIDsD3D11 &microlines_storage,
            PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids,
            uint64_t tiles_d3d11_buffer_id,
            uint64_t z_buffer_id) {
        // What will be the output of this function.
        auto fill_vertex_buffer_id = DeviceGl::allocate_general_buffer<Fill>(allocated_fill_count);

        // Upload fill indirect draw params to header of the Z-buffer.
        // This is in the Z-buffer, not its own buffer, to work around the 8 SSBO limitation on
        // some drivers (#373).
        uint32_t indirect_draw_params[8] = {6, 0, 0, 0, 0, microlines_storage.count, 0, 0};

        DeviceGl::upload_to_general_buffer<uint32_t>(z_buffer_id,
                                                      0,
                                                      indirect_draw_params,
                                                      8);

        bin_program->use();

        // Set uniforms.
        bin_program->set_int("uMicrolineCount", microlines_storage.count);
        bin_program->set_int("uMaxFillCount", allocated_fill_count);

        // Bind storage buffers.
        bin_program->bind_general_buffer(0, microlines_storage.buffer_id);
        bin_program->bind_general_buffer(1, propagate_metadata_buffer_ids.propagate_metadata);
        bin_program->bind_general_buffer(2, z_buffer_id);
        bin_program->bind_general_buffer(3, fill_vertex_buffer_id);
        bin_program->bind_general_buffer(4, tiles_d3d11_buffer_id);
        bin_program->bind_general_buffer(5, propagate_metadata_buffer_ids.backdrops);

        bin_program->dispatch((microlines_storage.count + BIN_WORKGROUP_SIZE - 1) / BIN_WORKGROUP_SIZE);

        DeviceGl::read_general_buffer<uint32_t>(
                z_buffer_id,
                0,
                indirect_draw_params,
                8);

        // How many fills we need.
        auto needed_fill_count = indirect_draw_params[FILL_INDIRECT_DRAW_PARAMS_INSTANCE_COUNT_INDEX];

        // If we didn't allocate enough space for the needed fills, we need to call this function again.
        if (needed_fill_count > allocated_fill_count) {
            allocated_fill_count = upper_power_of_two(needed_fill_count);

            // We need a larger buffer, but we should free the old one first.
            DeviceGl::free_general_buffer(fill_vertex_buffer_id);

            return {};
        }

        return {fill_vertex_buffer_id};
    }

    PropagateTilesInfoD3D11 RendererD3D11::propagate_tiles(
            uint32_t column_count,
            uint64_t tiles_d3d11_buffer_id,
            uint64_t z_buffer_id,
            uint64_t first_tile_map_buffer_id,
            uint64_t alpha_tiles_buffer_id,
            PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids) {
        // TODO(pcwalton): Zero out the Z-buffer on GPU?
        auto z_buffer_size = tile_size();
        auto tile_area = z_buffer_size.area();
        auto z_buffer_data = std::vector<int32_t>(tile_area, 0);
        DeviceGl::upload_to_general_buffer<int32_t>(
                z_buffer_id,
                0,
                z_buffer_data.data(),
                tile_area);

        // TODO(pcwalton): Initialize the first tiles buffer on GPU?
        auto first_tile_map = std::vector<FirstTileD3D11>(tile_area, FirstTileD3D11());
        DeviceGl::upload_to_general_buffer<FirstTileD3D11>(
                first_tile_map_buffer_id,
                0,
                first_tile_map.data(),
                tile_area);

        propagate_program->use();

        auto framebuffer_tile_size0 = framebuffer_tile_size();
        propagate_program->set_vec2i("uFramebufferTileSize", framebuffer_tile_size0.x, framebuffer_tile_size0.y);
        propagate_program->set_int("uColumnCount", column_count);
        propagate_program->set_int("uFirstAlphaTileIndex", alpha_tile_count);

        // Bind buffers.
        propagate_program->bind_general_buffer(0, propagate_metadata_buffer_ids.propagate_metadata);
        propagate_program->bind_general_buffer(1, propagate_metadata_buffer_ids.propagate_metadata);
        propagate_program->bind_general_buffer(2, propagate_metadata_buffer_ids.backdrops);
        propagate_program->bind_general_buffer(3, tiles_d3d11_buffer_id);
        propagate_program->bind_general_buffer(4, tiles_d3d11_buffer_id);
        propagate_program->bind_general_buffer(5, z_buffer_id);
        propagate_program->bind_general_buffer(6, first_tile_map_buffer_id);
        propagate_program->bind_general_buffer(7, alpha_tiles_buffer_id);

        propagate_program->dispatch((column_count + PROPAGATE_WORKGROUP_SIZE - 1) / PROPAGATE_WORKGROUP_SIZE);

        uint32_t fill_indirect_draw_params[8];
        DeviceGl::read_general_buffer<uint32_t>(
                z_buffer_id,
                0,
                fill_indirect_draw_params,
                8);

        auto batch_alpha_tile_count = fill_indirect_draw_params[FILL_INDIRECT_DRAW_PARAMS_ALPHA_TILE_COUNT_INDEX];

        auto alpha_tile_start = alpha_tile_count;
        alpha_tile_count += batch_alpha_tile_count;
        auto alpha_tile_end = alpha_tile_count;

        return {Range(alpha_tile_start, alpha_tile_end)};
    }

    Vec2<int> RendererD3D11::framebuffer_tile_size() {
        return pixel_size_to_tile_size(viewport_size);
    }

    void RendererD3D11::draw_fills(FillBufferInfoD3D11 &fill_storage_info,
                                   uint64_t tiles_d3d11_buffer_id,
                                   uint64_t alpha_tiles_buffer_id,
                                   PropagateTilesInfoD3D11 &propagate_tiles_info) {
        auto alpha_tile_range = propagate_tiles_info.alpha_tile_range;

        // This setup is a workaround for the annoying 64K limit of compute invocation in OpenGL.
        auto local_alpha_tile_count = alpha_tile_range.end - alpha_tile_range.start;

        fill_program->use();

        // Bind textures.
        fill_program->bind_texture(0, "uAreaLUT", area_lut_texture->get_texture_id());

        // Bind dest image.
        // We need to use imageLoad if we do clip.
        fill_program->bind_image(0, mask_texture->get_texture_id(), GL_READ_WRITE, GL_RGBA8);

        // Set uniforms.
        fill_program->set_vec2i("uAlphaTileRange",
                                alpha_tile_range.start,
                                alpha_tile_range.end);

        // Bind storage buffers.
        fill_program->bind_general_buffer(0, fill_storage_info.fill_vertex_buffer_id);
        fill_program->bind_general_buffer(1, tiles_d3d11_buffer_id);
        fill_program->bind_general_buffer(2, alpha_tiles_buffer_id);

        fill_program->dispatch(std::min(local_alpha_tile_count, (unsigned long long) 1 << 15),
                               ((local_alpha_tile_count + (1 << 15) - 1) >> 15));
    }

    void RendererD3D11::sort_tiles(uint64_t tiles_d3d11_buffer_id,
                                   uint64_t first_tile_map_buffer_id,
                                   uint64_t z_buffer_id) {
        auto tile_count = framebuffer_tile_size().area();

        sort_program->use();

        // Set uniforms.
        sort_program->set_int("uTileCount", tile_count);

        // Bind buffers.
        sort_program->bind_general_buffer(0, tiles_d3d11_buffer_id);
        sort_program->bind_general_buffer(1, first_tile_map_buffer_id);
        sort_program->bind_general_buffer(2, z_buffer_id);

        sort_program->dispatch((tile_count + SORT_WORKGROUP_SIZE - 1) / SORT_WORKGROUP_SIZE);
    }
}

#endif
