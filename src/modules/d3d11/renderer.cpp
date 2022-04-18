#include "renderer.h"

#include "gpu_data.h"
#include "../d3dx/data/data.h"
#include "../d3d9/data/draw_tile_batch.h"
#include "../../common/math/basic.h"
#include "../../common/io.h"
#include "../../gpu/platform.h"
#include "../../gpu/gl/command_buffer.h"
#include "../../common/logger.h"
#include "../../common/timestamp.h"

#include <array>

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

    Vec2<uint32_t> pixel_size_to_tile_size(Vec2<uint32_t> pixel_size) {
        // Round up.
        auto tile_size = Vec2<uint32_t>(TILE_WIDTH - 1, TILE_HEIGHT - 1);
        auto size = pixel_size + tile_size;
        return {size.x / TILE_WIDTH, size.y / TILE_HEIGHT};
    }

    void TileBatchInfoD3D11::clean() {
        z_buffer_id.reset();
        tiles_d3d11_buffer_id.reset();
        propagate_metadata_buffer_id.reset();
        first_tile_map_buffer_id.reset();
    }

    void SceneSourceBuffers::upload(SegmentsD3D11 &segments) {
        auto device = Platform::get_singleton().device;

        auto needed_points_capacity = upper_power_of_two(segments.points.size());
        auto needed_point_indices_capacity = upper_power_of_two(segments.indices.size());

        // Reallocate if capacity is not enough.
        if (points_capacity < needed_points_capacity) {
            // Old buffer will be dropped automatically.
            points_buffer = device->create_buffer(BufferType::General, needed_points_capacity * sizeof(Vec2<float>));

            points_capacity = needed_points_capacity;
        }

        // Reallocate if capacity is not enough.
        if (point_indices_capacity < needed_point_indices_capacity) {
            // Old buffer will be dropped automatically.
            point_indices_buffer = device->create_buffer(BufferType::General,
                                                         needed_point_indices_capacity * sizeof(SegmentIndicesD3D11));

            point_indices_capacity = needed_point_indices_capacity;
        }

        point_indices_count = segments.indices.size();

        // Upload data.
        {
            auto cmd_buffer = device->create_command_buffer();

            cmd_buffer->upload_to_buffer(points_buffer,
                                         0,
                                         segments.points.size() * sizeof(Vec2<float>),
                                         segments.points.data());

            cmd_buffer->upload_to_buffer(point_indices_buffer,
                                         0,
                                         segments.indices.size() * sizeof(SegmentIndicesD3D11),
                                         segments.indices.data());

            cmd_buffer->submit();
        }
    }

    SceneBuffers::~SceneBuffers() {
        //device->free_general_buffer(draw.points_buffer);
        //device->free_general_buffer(draw.point_indices_buffer);
    }

    void SceneBuffers::upload(SegmentsD3D11 &draw_segments, SegmentsD3D11 &clip_segments) {
        draw.upload(draw_segments);
        //clip.upload(clip_segments);
    }

    RendererD3D11::RendererD3D11(uint32_t canvas_width, uint32_t canvas_height) {
        auto device = Platform::get_singleton().device;

        allocated_microline_count = INITIAL_ALLOCATED_MICROLINE_COUNT;
        allocated_fill_count = INITIAL_ALLOCATED_FILL_COUNT;

        // Create uniform buffers.
        bin_ub = device->create_buffer(BufferType::Uniform, 4 * sizeof(int32_t));
        bound_ub = device->create_buffer(BufferType::Uniform, 4 * sizeof(int32_t));
        dice_ub0 = device->create_buffer(BufferType::Uniform, 12 * sizeof(float));
        dice_ub1 = device->create_buffer(BufferType::Uniform, 4 * sizeof(int32_t));
        fill_ub = device->create_buffer(BufferType::Uniform, 4 * sizeof(int32_t));
        propagate_ub = device->create_buffer(BufferType::Uniform, 4 * sizeof(int32_t));
        sort_ub = device->create_buffer(BufferType::Uniform, 4 * sizeof(int32_t));
        tile_ub0 = device->create_buffer(BufferType::Uniform, 8 * sizeof(float));
        tile_ub1 = device->create_buffer(BufferType::Uniform, 8 * sizeof(float));

        // Unlike D3D9, we use RGBA8 here instead of RGBA16F.
        mask_texture = device->create_texture(MASK_FRAMEBUFFER_WIDTH,
                                              MASK_FRAMEBUFFER_HEIGHT,
                                              TextureFormat::RGBA8,
                                              DataType::UNSIGNED_BYTE);

        dest_texture = device->create_texture(canvas_width,
                                              canvas_height,
                                              TextureFormat::RGBA8,
                                              DataType::UNSIGNED_BYTE);
    }

    void RendererD3D11::set_up_pipelines() {
        auto device = Platform::get_singleton().device;

#ifdef PATHFINDER_SHADERS_EMBEDDED
        const std::string dice_string =
#include "../src/shaders/minified/minified_dice.comp"
        ;
        const std::string bound_string =
#include "../src/shaders/minified/minified_bound.comp"
        ;
        const std::string bin_string =
#include "../src/shaders/minified/minified_bin.comp"
        ;
        const std::string propagate_string =
#include "../src/shaders/minified/minified_propagate.comp"
        ;
        const std::string sort_string =
#include "../src/shaders/minified/minified_sort.comp"
        ;
        const std::string fill_string =
#include "../src/shaders/minified/minified_fill.comp"
        ;
        const std::string tile_string_0 =
#include "../src/shaders/minified/minified_tile.comp.0"
        ;
        const std::string tile_string_1 =
#include "../src/shaders/minified/minified_tile.comp.1"
        ;
        const std::string tile_string = tile_string_0 + tile_string_1;

        const std::vector<char> dice_source = {dice_string.begin(), dice_string.end()};
        const std::vector<char> bound_source = {bound_string.begin(), bound_string.end()};
        const std::vector<char> bin_source = {bin_string.begin(), bin_string.end()};
        const std::vector<char> propagate_source = {propagate_string.begin(), propagate_string.end()};
        const std::vector<char> fill_source = {fill_string.begin(), fill_string.end()};
        const std::vector<char> sort_source = {sort_string.begin(), sort_string.end()};
        const std::vector<char> tile_source = {tile_string.begin(), tile_string.end()};
#else
        const auto dice_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d11/dice.comp");
        const auto bound_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d11/bound.comp");
        const auto bin_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d11/bin.comp");
        const auto propagate_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d11/propagate.comp");
        const auto fill_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d11/fill.comp");
        const auto sort_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d11/sort.comp");
        const auto tile_source = load_file_as_bytes(PATHFINDER_SHADER_DIR"d3d11/tile.comp");
#endif

        dice_pipeline = device->create_compute_pipeline(dice_source); // 1
        bound_pipeline = device->create_compute_pipeline(bound_source); // 2
        bin_pipeline = device->create_compute_pipeline(bin_source); // 3
        propagate_pipeline = device->create_compute_pipeline(propagate_source); // 4
        fill_pipeline = device->create_compute_pipeline(fill_source); // 5
        sort_pipeline = device->create_compute_pipeline(sort_source); // 6
        tile_pipeline = device->create_compute_pipeline(tile_source); // 7

        // Bound pipeline.
        {
            bound_descriptor_set = std::make_shared<DescriptorSet>();

            bound_descriptor_set->add_or_update_descriptor({DescriptorType::UniformBuffer, 0, "bUniform", bound_ub, nullptr});
        }

        // Dice pipeline.
        {
            dice_descriptor_set = std::make_shared<DescriptorSet>();

            dice_descriptor_set->add_or_update_descriptor({DescriptorType::UniformBuffer, 0, "bUniform0", dice_ub0, nullptr});
            dice_descriptor_set->add_or_update_descriptor({DescriptorType::UniformBuffer, 1, "bUniform1", dice_ub1, nullptr});
        }

        // Bin pipeline.
        {
            bin_descriptor_set = std::make_shared<DescriptorSet>();

            bin_descriptor_set->add_or_update_descriptor({DescriptorType::UniformBuffer, 0, "bUniform", bin_ub, nullptr});
        }

        // Propagate pipeline.
        {
            propagate_descriptor_set = std::make_shared<DescriptorSet>();

            propagate_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::UniformBuffer, 0, "bUniform", propagate_ub, nullptr});
        }

        // Sort pipeline.
        {
            sort_descriptor_set = std::make_shared<DescriptorSet>();

            sort_descriptor_set->add_or_update_descriptor({DescriptorType::UniformBuffer, 0, "bUniform", sort_ub, nullptr});
        }

        // Fill pipeline.
        {
            fill_descriptor_set = std::make_shared<DescriptorSet>();

            fill_descriptor_set->add_or_update_descriptor({DescriptorType::UniformBuffer, 0, "bUniform", fill_ub, nullptr});

            fill_descriptor_set->add_or_update_descriptor({DescriptorType::Texture, 0, "uAreaLUT", nullptr, area_lut_texture});
        }

        // Tile pipeline.
        {
            // Set descriptor set.
            {
                tile_descriptor_set = std::make_shared<DescriptorSet>();

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::UniformBuffer;
                    descriptor.binding = 0;
                    descriptor.binding_name = "bFixedUniform";
                    descriptor.buffer = fixed_sizes_ub;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::UniformBuffer;
                    descriptor.binding = 1;
                    descriptor.binding_name = "bUniform0";
                    descriptor.buffer = tile_ub0;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::UniformBuffer;
                    descriptor.binding = 2;
                    descriptor.binding_name = "bUniform1";
                    descriptor.buffer = tile_ub1;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::Texture;
                    descriptor.binding = 0;
                    descriptor.binding_name = "uTextureMetadata";
                    descriptor.texture = metadata_texture;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }

                {
                    Descriptor descriptor;
                    descriptor.type = DescriptorType::Texture;
                    descriptor.binding = 3;
                    descriptor.binding_name = "uMaskTexture0";
                    descriptor.texture = mask_texture;

                    tile_descriptor_set->add_or_update_descriptor(descriptor);
                }
            }
        }
    }

    void RendererD3D11::draw(SceneBuilderD3D11 &scene_builder) {
        if (scene_builder.built_segments.draw_segments.points.empty()) return;

        // RenderCommand::UploadSceneD3D11
        upload_scene(scene_builder.built_segments.draw_segments, scene_builder.built_segments.clip_segments);

        alpha_tile_count = 0;
        need_to_clear_dest = true;

        for (auto &batch: scene_builder.tile_batches) {
            prepare_and_draw_tiles(batch, scene_builder.metadata);
        }
    }

    std::shared_ptr<Texture> RendererD3D11::get_dest_texture() {
        return dest_texture;
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

        // Free general buffers in batch_info.
        batch_info.clean();
    }

    void RendererD3D11::draw_tiles(const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
                                   const std::shared_ptr<Buffer> &first_tile_map_buffer_id,
                                   const RenderTarget &render_target,
                                   const RenderTarget &color_target) {
        auto device = Platform::get_singleton().device;

        // The framebuffer mentioned here is different from the target viewport.
        // This doesn't change as long as the destination texture's size doesn't change.
        auto framebuffer_tile_size0 = framebuffer_tile_size();

        // Decide render target.
        Vec2<uint32_t> target_size;
        std::shared_ptr<Texture> target_texture;
        int clear_op;
        // If no specific RenderTarget is given, we render to the destination texture.
        if (render_target.framebuffer == nullptr) {
            target_size = dest_texture->get_size();
            target_texture = dest_texture;
            clear_op = need_to_clear_dest ? LOAD_ACTION_CLEAR : LOAD_ACTION_LOAD;
            need_to_clear_dest = false;
        } else {
            target_size = render_target.framebuffer->get_size();
            target_texture = render_target.framebuffer->get_texture();
            clear_op = LOAD_ACTION_CLEAR;
        }

        // Update uniform buffers.
        {
            std::array<float, 8> ubo_data0 = {0, 0, 0, 0, // uClearColor
                                              (float) color_target.size.x,
                                              (float) color_target.size.y, // uColorTextureSize0
                                              (float) target_size.x, (float) target_size.y}; // uFramebufferSize


            std::array<int32_t, 5> ubo_data1 = {0, 0, // uZBufferSize
                                                (int32_t) framebuffer_tile_size0.x,
                                                (int32_t) framebuffer_tile_size0.y, // uFramebufferTileSize
                                                clear_op}; // uLoadAction

            auto one_shot_cmd_buffer = device->create_command_buffer();
            one_shot_cmd_buffer->upload_to_buffer(tile_ub0, 0, 8 * sizeof(float), ubo_data0.data());
            one_shot_cmd_buffer->upload_to_buffer(tile_ub1, 0, 5 * sizeof(int32_t), ubo_data1.data());
            one_shot_cmd_buffer->submit();
        }

        // Update descriptor set.
        {
            tile_descriptor_set->add_or_update_descriptor({DescriptorType::Texture, 1, "uZBuffer", nullptr, mask_texture});
            if (color_target.framebuffer != nullptr) {
                tile_descriptor_set->add_or_update_descriptor({DescriptorType::Texture, 2, "uColorTexture0", nullptr,
                                                     color_target.framebuffer->get_texture()});
            }
            tile_descriptor_set->add_or_update_descriptor({DescriptorType::Texture, 4, "uGammaLUT", nullptr, nullptr});

            tile_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 0, "", tiles_d3d11_buffer_id, nullptr}); // Read only.
            tile_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 1, "", first_tile_map_buffer_id, nullptr}); // Read only.

            tile_descriptor_set->add_or_update_descriptor({DescriptorType::Image, 0, "", nullptr, target_texture});
        }

        auto cmd_buffer = device->create_command_buffer();

        cmd_buffer->begin_compute_pass();

        cmd_buffer->bind_compute_pipeline(tile_pipeline);

        cmd_buffer->bind_descriptor_set(tile_descriptor_set);

        cmd_buffer->dispatch(framebuffer_tile_size0.x, framebuffer_tile_size0.y, 1);

        cmd_buffer->end_compute_pass();

        cmd_buffer->submit();
    }

    Vec2<uint32_t> RendererD3D11::tile_size() const {
        auto temp = dest_texture->get_size() + Vec2<uint32_t>(TILE_WIDTH - 1, TILE_HEIGHT - 1);
        return {temp.x / TILE_WIDTH, temp.y / TILE_HEIGHT};
    }

    std::shared_ptr<Buffer> RendererD3D11::allocate_z_buffer() {
        auto device = Platform::get_singleton().device;

        // This includes the fill indirect draw params because some drivers limit the number of
        // SSBOs to 8 (#373).
        // Add FILL_INDIRECT_DRAW_PARAMS_SIZE in case tile size is zero.
        auto size = tile_size().area() + FILL_INDIRECT_DRAW_PARAMS_SIZE;
        auto buffer_id = device->create_buffer(BufferType::General, size * sizeof(int32_t));

        return buffer_id;
    }

    std::shared_ptr<Buffer> RendererD3D11::allocate_first_tile_map() {
        auto device = Platform::get_singleton().device;

        auto size = tile_size().area();
        auto buffer_id = device->create_buffer(BufferType::General, size * sizeof(FirstTileD3D11));

        return buffer_id;
    }

    std::shared_ptr<Buffer> RendererD3D11::allocate_alpha_tile_info(uint32_t index_count) {
        auto device = Platform::get_singleton().device;

        auto buffer_id = device->create_buffer(BufferType::General, index_count * sizeof(AlphaTileD3D11));

        return buffer_id;
    }

    PropagateMetadataBufferIDsD3D11 RendererD3D11::upload_propagate_metadata(
            std::vector<PropagateMetadataD3D11> &propagate_metadata,
            std::vector<BackdropInfoD3D11> &backdrops) {
        auto device = Platform::get_singleton().device;

        auto propagate_metadata_storage_id =
                device->create_buffer(BufferType::General,
                                      propagate_metadata.size() * sizeof(PropagateMetadataD3D11));

        auto one_shot_cmd_buffer = device->create_command_buffer();
        one_shot_cmd_buffer->upload_to_buffer(propagate_metadata_storage_id,
                                     0,
                                     propagate_metadata.size() * sizeof(PropagateMetadataD3D11),
                                     propagate_metadata.data());
        one_shot_cmd_buffer->submit();

        auto backdrops_storage_id = device->create_buffer(BufferType::General,
                                                          backdrops.size() * sizeof(BackdropInfoD3D11));

        return {propagate_metadata_storage_id, backdrops_storage_id};
    }

    void RendererD3D11::upload_initial_backdrops(const std::shared_ptr<Buffer> &backdrops_buffer_id,
                                                 std::vector<BackdropInfoD3D11> &backdrops) {
        auto device = Platform::get_singleton().device;

        auto one_shot_cmd_buffer = device->create_command_buffer();
        one_shot_cmd_buffer->upload_to_buffer(backdrops_buffer_id,
                                     0,
                                     backdrops.size() * sizeof(BackdropInfoD3D11),
                                     backdrops.data());
        one_shot_cmd_buffer->submit();
    }

    void RendererD3D11::prepare_tiles(TileBatchDataD3D11 &batch) {
        auto device = Platform::get_singleton().device;

        // Upload tiles to GPU or allocate them as appropriate.
        auto tiles_d3d11_buffer_id = device->create_buffer(BufferType::General,
                                                           batch.tile_count * sizeof(TileD3D11));

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
        for (int _ = 0; _ < 2; _++) {
            microlines_storage = dice_segments(
                    batch.prepare_info.dice_metadata,
                    batch.segment_count,
                    batch.path_source,
                    batch.prepare_info.transform);

            // If the microlines buffer has been allocated successfully.
            if (microlines_storage.buffer_id != nullptr)
                break;
        }
        if (microlines_storage.buffer_id == nullptr) {
            Logger::error("Ran out of space for microlines when dicing!", "D3D9");
        }

        // Initialize tiles, bin segments. We might have to do this twice if our first
        // attempt runs out of space in the fill buffer. If this is the case, we also
        // need to re-initialize tiles and re-upload backdrops because they would have
        // been modified during the first attempt.
        FillBufferInfoD3D11 fill_buffer_info{};
        for (int _ = 0; _ < 2; _++) {
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
            if (fill_buffer_info.fill_vertex_buffer_id != nullptr) {
                break;
            }
        }
        if (fill_buffer_info.fill_vertex_buffer_id == nullptr) {
            Logger::error("Ran out of space for fills when binning!", "D3D9");
        }

        // Free microlines storage as it's not needed anymore.
        microlines_storage.buffer_id.reset();

        // TODO(pcwalton): If we run out of space for alpha tile indices, propagate multiple times.

        auto alpha_tiles_buffer_id = allocate_alpha_tile_info(batch.tile_count);

        auto propagate_tiles_info = propagate_tiles(
                batch.prepare_info.backdrops.size(),
                tiles_d3d11_buffer_id,
                z_buffer_id,
                first_tile_map_buffer_id,
                alpha_tiles_buffer_id,
                propagate_metadata_buffer_ids);

        propagate_metadata_buffer_ids.backdrops.reset();

        draw_fills(fill_buffer_info,
                   tiles_d3d11_buffer_id,
                   alpha_tiles_buffer_id,
                   propagate_tiles_info);

        fill_buffer_info.fill_vertex_buffer_id.reset();
        alpha_tiles_buffer_id.reset();

        // FIXME(pcwalton): This seems like the wrong place to do this...
        sort_tiles(tiles_d3d11_buffer_id, first_tile_map_buffer_id, z_buffer_id);

        // Record tile batch info.
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
        auto device = Platform::get_singleton().device;

        // Allocate some general buffers.
        auto microlines_buffer_id = device->create_buffer(BufferType::General,
                                                          allocated_microline_count * sizeof(MicrolineD3D11));
        auto dice_metadata_buffer_id = device->create_buffer(BufferType::General,
                                                             dice_metadata.size() * sizeof(DiceMetadataD3D11));
        auto indirect_draw_params_buffer_id = device->create_buffer(BufferType::General,
                                                                    8 * sizeof(uint32_t));

        // Get scene source buffers.
        auto &scene_source_buffers = path_source == PathSource::Draw ? scene_buffers.draw : scene_buffers.clip;
        auto points_buffer_id = scene_source_buffers.points_buffer;
        auto point_indices_buffer_id = scene_source_buffers.point_indices_buffer;
        auto point_indices_count = scene_source_buffers.point_indices_count;

        // Upload dice indirect draw params, which are also used for output.
        uint32_t indirect_compute_params[8] = {0, 0, 0, 0, point_indices_count, 0, 0, 0};

        auto one_shot_cmd_buffer = device->create_command_buffer();
        one_shot_cmd_buffer->upload_to_buffer(
                indirect_draw_params_buffer_id,
                0,
                8 * sizeof(uint32_t),
                indirect_compute_params);

        // Upload dice metadata.
        one_shot_cmd_buffer->upload_to_buffer(
                dice_metadata_buffer_id,
                0,
                dice_metadata.size() * sizeof(DiceMetadataD3D11),
                dice_metadata.data());

        // Update uniform buffers.
        {
            // Note that a row of mat2 occupies 4 floats just like a mat4.
            std::array<float, 10> ubo_data0 = {transform.matrix.v[0], transform.matrix.v[1], 0, 0,
                                               transform.matrix.v[2], transform.matrix.v[3], 0, 0,
                                               transform.vector.x, transform.vector.y};
            one_shot_cmd_buffer->upload_to_buffer(dice_ub0, 0, 10 * sizeof(float), ubo_data0.data());

            std::array<int32_t, 3> ubo_data1 = {static_cast<int32_t>(dice_metadata.size()),
                                                static_cast<int32_t>(batch_segment_count),
                                                static_cast<int32_t>(allocated_microline_count)};
            one_shot_cmd_buffer->upload_to_buffer(dice_ub1, 0, 3 * sizeof(int32_t), ubo_data1.data());
        }

        one_shot_cmd_buffer->submit();

        // Bind storage buffers.
        {
            dice_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 0, "", indirect_draw_params_buffer_id, nullptr}); // Read write.
            dice_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 1, "", dice_metadata_buffer_id, nullptr}); // Read only.
            dice_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 2, "", points_buffer_id, nullptr}); // Read only.
            dice_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 3, "", point_indices_buffer_id, nullptr}); // Read only.
            dice_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 4, "", microlines_buffer_id, nullptr}); // Write only.
        }

        auto cmd_buffer = device->create_command_buffer();

        cmd_buffer->begin_compute_pass();

        cmd_buffer->bind_compute_pipeline(dice_pipeline);

        cmd_buffer->bind_descriptor_set(dice_descriptor_set);

        cmd_buffer->dispatch((batch_segment_count + DICE_WORKGROUP_SIZE - 1) / DICE_WORKGROUP_SIZE, 1, 1);

        cmd_buffer->end_compute_pass();

        cmd_buffer->submit();

        // Read indirect draw params back to CPU memory.
        one_shot_cmd_buffer = device->create_command_buffer();
        one_shot_cmd_buffer->read_buffer(indirect_draw_params_buffer_id,
                                         0,
                                         8 * sizeof(uint32_t),
                                         indirect_compute_params);
        one_shot_cmd_buffer->submit();

        // Free some general buffers which are no longer useful.
        //device->free_general_buffer(dice_metadata_buffer_id);
        //device->free_general_buffer(indirect_draw_params_buffer_id);

        // Fetch microline count from indirect draw params.
        auto microline_count = indirect_compute_params[BIN_INDIRECT_DRAW_PARAMS_MICROLINE_COUNT_INDEX];

        // Allocate more space if not allocated enough.
        if (microline_count > allocated_microline_count) {
            allocated_microline_count = upper_power_of_two(microline_count);

            // We need a larger buffer, but we should free the old one first.
            //device->free_general_buffer(microlines_buffer_id);

            return {{}, 0};
        }

        return {microlines_buffer_id, microline_count};
    }

    void RendererD3D11::bound(const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
                              uint32_t tile_count,
                              std::vector<TilePathInfoD3D11> &tile_path_info) {
        auto device = Platform::get_singleton().device;

        auto one_shot_cmd_buffer = device->create_command_buffer();

        // This is a staging buffer, which will be freed in the end of this function.
        auto path_info_buffer_id = device->create_buffer(BufferType::General,
                                                         tile_path_info.size() * sizeof(TilePathInfoD3D11));
        one_shot_cmd_buffer->upload_to_buffer(path_info_buffer_id,
                                              0,
                                              tile_path_info.size() * sizeof(TilePathInfoD3D11),
                                              tile_path_info.data());

        // Update uniform buffers.
        std::array<int32_t, 2> ubo_data = {static_cast<int32_t>(tile_path_info.size()),
                                           static_cast<int32_t>(tile_count)};
        one_shot_cmd_buffer->upload_to_buffer(bound_ub, 0, 2 * sizeof(int32_t), ubo_data.data());

        one_shot_cmd_buffer->submit();

        // Bind storage buffers.
        {
            bound_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 0, "", path_info_buffer_id, nullptr}); // Read only.
            bound_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 1, "", tiles_d3d11_buffer_id, nullptr}); // Write only.
        }

        auto cmd_buffer = device->create_command_buffer();

        cmd_buffer->begin_compute_pass();

        cmd_buffer->bind_compute_pipeline(bound_pipeline);

        cmd_buffer->bind_descriptor_set(bound_descriptor_set);

        cmd_buffer->dispatch((tile_count + BOUND_WORKGROUP_SIZE - 1) / BOUND_WORKGROUP_SIZE, 1, 1);

        cmd_buffer->end_compute_pass();

        cmd_buffer->submit();

        //device->free_general_buffer(path_info_buffer_id);
    }

    FillBufferInfoD3D11 RendererD3D11::bin_segments(
            MicrolinesBufferIDsD3D11 &microlines_storage,
            PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids,
            const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
            const std::shared_ptr<Buffer> &z_buffer_id) {
        auto device = Platform::get_singleton().device;

        auto one_shot_cmd_buffer = device->create_command_buffer();

        // What will be the output of this function.
        auto fill_vertex_buffer_id = device->create_buffer(BufferType::General,
                                                           allocated_fill_count * sizeof(Fill));

        // Upload fill indirect draw params to header of the Z-buffer.
        // This is in the Z-buffer, not its own buffer, to work around the 8 SSBO limitation on
        // some drivers (#373).
        uint32_t indirect_draw_params[8] = {6, 0, 0, 0, 0, microlines_storage.count, 0, 0};

        one_shot_cmd_buffer->upload_to_buffer(z_buffer_id,
                                              0,
                                              8 * sizeof(uint32_t),
                                              indirect_draw_params);

        // Update uniform buffers.
        std::array<int32_t, 2> ubo_data = {(int32_t) microlines_storage.count, (int32_t) allocated_fill_count};
        one_shot_cmd_buffer->upload_to_buffer(bin_ub, 0, 2 * sizeof(int32_t), ubo_data.data());

        one_shot_cmd_buffer->submit();

        // Bind storage buffers.
        {
            bin_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 0, "", microlines_storage.buffer_id, nullptr}); // Read only.
            bin_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 1, "", propagate_metadata_buffer_ids.propagate_metadata,
                     nullptr}); // Read only.
            bin_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 2, "", z_buffer_id, nullptr}); // Read write.
            bin_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 3, "", fill_vertex_buffer_id, nullptr}); // Write only.
            bin_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 4, "", tiles_d3d11_buffer_id, nullptr}); // Read write.
            bin_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 5, "", propagate_metadata_buffer_ids.backdrops,
                     nullptr}); // Read write.
        }

        auto cmd_buffer = device->create_command_buffer();

        cmd_buffer->begin_compute_pass();

        cmd_buffer->bind_compute_pipeline(bin_pipeline);

        cmd_buffer->bind_descriptor_set(bin_descriptor_set);

        cmd_buffer->dispatch((microlines_storage.count + BIN_WORKGROUP_SIZE - 1) / BIN_WORKGROUP_SIZE , 1, 1);

        cmd_buffer->end_compute_pass();

        cmd_buffer->submit();

        one_shot_cmd_buffer = device->create_command_buffer();
        one_shot_cmd_buffer->read_buffer(z_buffer_id,
                                         0,
                                         8 * sizeof(uint32_t),
                                         indirect_draw_params);
        one_shot_cmd_buffer->submit();

        // How many fills we need.
        auto needed_fill_count = indirect_draw_params[FILL_INDIRECT_DRAW_PARAMS_INSTANCE_COUNT_INDEX];

        // If we didn't allocate enough space for the needed fills, we need to call this function again.
        if (needed_fill_count > allocated_fill_count) {
            allocated_fill_count = upper_power_of_two(needed_fill_count);

            // We need a larger buffer, but we should free the old one first.
            //device->free_general_buffer(fill_vertex_buffer_id);

            return {};
        }

        return {fill_vertex_buffer_id};
    }

    PropagateTilesInfoD3D11 RendererD3D11::propagate_tiles(
            uint32_t column_count,
            const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
            const std::shared_ptr<Buffer> &z_buffer_id,
            const std::shared_ptr<Buffer> &first_tile_map_buffer_id,
            const std::shared_ptr<Buffer> &alpha_tiles_buffer_id,
            PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids) {
        auto device = Platform::get_singleton().device;

        auto one_shot_cmd_buffer = device->create_command_buffer();

        // TODO(pcwalton): Zero out the Z-buffer on GPU?
        auto z_buffer_size = tile_size();
        auto tile_area = z_buffer_size.area();
        auto z_buffer_data = std::vector<int32_t>(tile_area, 0);
        one_shot_cmd_buffer->upload_to_buffer(z_buffer_id,
                                              0,
                                              tile_area * sizeof(int32_t),
                                              z_buffer_data.data());

        // TODO(pcwalton): Initialize the first tiles buffer on GPU?
        auto first_tile_map = std::vector<FirstTileD3D11>(tile_area, FirstTileD3D11());
        one_shot_cmd_buffer->upload_to_buffer(first_tile_map_buffer_id,
                                              0,
                                              tile_area * sizeof(FirstTileD3D11),
                                              first_tile_map.data());

        // Update uniform buffers.
        auto framebuffer_tile_size0 = framebuffer_tile_size();
        std::array<int32_t, 4> ubo_data = {(int32_t) framebuffer_tile_size0.x,
                                           (int32_t) framebuffer_tile_size0.y,
                                           (int32_t) column_count,
                                           (int32_t) alpha_tile_count};
        one_shot_cmd_buffer->upload_to_buffer(propagate_ub, 0, 4 * sizeof(int32_t), ubo_data.data());

        one_shot_cmd_buffer->submit();

        // Bind storage buffers.
        {
            propagate_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 0, "", propagate_metadata_buffer_ids.propagate_metadata,
                     nullptr}); // Read only.
            propagate_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 1, "", nullptr, nullptr}); // Clip metadata, read only.
            propagate_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 2, "", propagate_metadata_buffer_ids.backdrops,
                     nullptr}); // Read only.
            propagate_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 3, "", tiles_d3d11_buffer_id, nullptr}); // Read write.
            propagate_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 4, "", nullptr, nullptr}); // Clip tiles, read write.
            propagate_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 5, "", z_buffer_id, nullptr}); // Read write.
            propagate_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 6, "", first_tile_map_buffer_id, nullptr}); // Read write.
            propagate_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 7, "", alpha_tiles_buffer_id, nullptr}); // Write only.
        }

        auto cmd_buffer = device->create_command_buffer();

        cmd_buffer->begin_compute_pass();

        cmd_buffer->bind_compute_pipeline(propagate_pipeline);

        cmd_buffer->bind_descriptor_set(propagate_descriptor_set);

        cmd_buffer->dispatch((column_count + PROPAGATE_WORKGROUP_SIZE - 1) / PROPAGATE_WORKGROUP_SIZE, 1, 1);

        cmd_buffer->end_compute_pass();

        cmd_buffer->submit();

        one_shot_cmd_buffer = device->create_command_buffer();

        uint32_t fill_indirect_draw_params[8];
        one_shot_cmd_buffer->read_buffer(z_buffer_id,
                                         0,
                                         8 * sizeof(uint32_t),
                                         fill_indirect_draw_params);

        one_shot_cmd_buffer->submit();

        auto batch_alpha_tile_count = fill_indirect_draw_params[FILL_INDIRECT_DRAW_PARAMS_ALPHA_TILE_COUNT_INDEX];

        auto alpha_tile_start = alpha_tile_count;
        alpha_tile_count += batch_alpha_tile_count;
        auto alpha_tile_end = alpha_tile_count;

        return {Range(alpha_tile_start, alpha_tile_end)};
    }

    Vec2<uint32_t> RendererD3D11::framebuffer_tile_size() {
        return pixel_size_to_tile_size(dest_texture->get_size());
    }

    void RendererD3D11::draw_fills(FillBufferInfoD3D11 &fill_storage_info,
                                   const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
                                   const std::shared_ptr<Buffer> &alpha_tiles_buffer_id,
                                   PropagateTilesInfoD3D11 &propagate_tiles_info) {
        auto device = Platform::get_singleton().device;

        auto alpha_tile_range = propagate_tiles_info.alpha_tile_range;

        // This setup is a workaround for the annoying 64K limit of compute invocation in OpenGL.
        uint32_t local_alpha_tile_count = alpha_tile_range.end - alpha_tile_range.start;

        auto one_shot_cmd_buffer = device->create_command_buffer();

        // Update uniform buffers.
        auto framebuffer_tile_size0 = framebuffer_tile_size();
        std::array<int32_t, 2> ubo_data = {static_cast<int32_t>(alpha_tile_range.start),
                                           static_cast<int32_t>(alpha_tile_range.end)};
        one_shot_cmd_buffer->upload_to_buffer(fill_ub, 0, 2 * sizeof(int32_t), ubo_data.data());

        one_shot_cmd_buffer->submit();

        // Update descriptor set.
        {
            fill_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 0, "", fill_storage_info.fill_vertex_buffer_id,
                     nullptr}); // Read only.
            fill_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 1, "", tiles_d3d11_buffer_id, nullptr}); // Read only.
            fill_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 2, "", alpha_tiles_buffer_id, nullptr}); // Read only.
            fill_descriptor_set->add_or_update_descriptor({DescriptorType::Image, 0, "", nullptr, mask_texture});
        }

        auto cmd_buffer = device->create_command_buffer();

        cmd_buffer->begin_compute_pass();

        cmd_buffer->bind_compute_pipeline(fill_pipeline);

        cmd_buffer->bind_descriptor_set(fill_descriptor_set);

        cmd_buffer->dispatch(std::min(local_alpha_tile_count, 1u << 15u),
                             (local_alpha_tile_count + (1 << 15) - 1) >> 15,
                             1);

        cmd_buffer->end_compute_pass();

        cmd_buffer->submit();
    }

    void RendererD3D11::sort_tiles(const std::shared_ptr<Buffer> &tiles_d3d11_buffer_id,
                                   const std::shared_ptr<Buffer> &first_tile_map_buffer_id,
                                   const std::shared_ptr<Buffer> &z_buffer_id) {
        auto device = Platform::get_singleton().device;

        auto tile_count = framebuffer_tile_size().area();

        auto one_shot_cmd_buffer = device->create_command_buffer();

        // Update uniform buffers.
        one_shot_cmd_buffer->upload_to_buffer(sort_ub, 0, sizeof(int32_t), &tile_count);

        one_shot_cmd_buffer->submit();

        // Update descriptor set.
        {
            sort_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 0, "", tiles_d3d11_buffer_id, nullptr}); // Read write.
            sort_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 1, "", first_tile_map_buffer_id, nullptr}); // Read write.
            sort_descriptor_set->add_or_update_descriptor(
                    {DescriptorType::GeneralBuffer, 2, "", z_buffer_id, nullptr}); // Read only.
        }

        auto cmd_buffer = device->create_command_buffer();

        cmd_buffer->begin_compute_pass();

        cmd_buffer->bind_compute_pipeline(sort_pipeline);

        cmd_buffer->bind_descriptor_set(sort_descriptor_set);

        cmd_buffer->dispatch((tile_count + SORT_WORKGROUP_SIZE - 1) / SORT_WORKGROUP_SIZE, 1, 1);

        cmd_buffer->end_compute_pass();

        cmd_buffer->submit();
    }
}

#endif
