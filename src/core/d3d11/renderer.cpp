#include "renderer.h"

#include <array>

#include "../../common/io.h"
#include "../../common/logger.h"
#include "../../common/math/basic.h"
#include "../../common/timestamp.h"
#include "../../gpu/command_buffer.h"
#include "../../gpu/window.h"
#include "../data/data.h"
#include "gpu_data.h"

#ifdef PATHFINDER_USE_D3D11
    #ifdef PATHFINDER_USE_VULKAN
        #include "../../shaders/generated/bin_comp_spv.h"
        #include "../../shaders/generated/bound_comp_spv.h"
        #include "../../shaders/generated/dice_comp_spv.h"
        #include "../../shaders/generated/fill_comp_spv.h"
        #include "../../shaders/generated/propagate_comp_spv.h"
        #include "../../shaders/generated/sort_comp_spv.h"
        #include "../../shaders/generated/tile_comp_spv.h"
    #else
        #include "../../shaders/generated/bin_comp.h"
        #include "../../shaders/generated/bound_comp.h"
        #include "../../shaders/generated/dice_comp.h"
        #include "../../shaders/generated/fill_comp.h"
        #include "../../shaders/generated/propagate_comp.h"
        #include "../../shaders/generated/sort_comp.h"
        #include "../../shaders/generated/tile_comp.h"
    #endif

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

Vec2I pixel_size_to_tile_size(Vec2I pixel_size) {
    // Round up.
    auto tile_size = Vec2I(TILE_WIDTH - 1, TILE_HEIGHT - 1);
    auto size = pixel_size + tile_size;
    return {size.x / TILE_WIDTH, size.y / TILE_HEIGHT};
}

void SceneSourceBuffers::upload(SegmentsD3D11 &segments,
                                const std::shared_ptr<Driver> &driver,
                                const std::shared_ptr<CommandBuffer> &cmd_buffer,
                                const std::shared_ptr<GpuMemoryAllocator> &allocator) {
    auto needed_points_capacity = upper_power_of_two(segments.points.size());
    auto needed_point_indices_capacity = upper_power_of_two(segments.indices.size());

    // Reallocate if capacity is not enough.
    if (points_capacity < needed_points_capacity) {
        if (points_buffer) {
            allocator->free_general_buffer(*points_buffer);
        }

        points_buffer = std::make_shared<uint64_t>(
            allocator->allocate_general_buffer(needed_points_capacity * sizeof(Vec2F), "Points buffer"));

        points_capacity = needed_points_capacity;
    }

    // Reallocate if capacity is not enough.
    if (point_indices_capacity < needed_point_indices_capacity) {
        if (point_indices_buffer) {
            allocator->free_general_buffer(*point_indices_buffer);
        }

        point_indices_buffer = std::make_shared<uint64_t>(
            allocator->allocate_general_buffer(needed_point_indices_capacity * sizeof(SegmentIndicesD3D11),
                                               "Point indices buffer"));

        point_indices_capacity = needed_point_indices_capacity;
    }

    point_indices_count = segments.indices.size();

    // Upload data.
    if (needed_points_capacity != 0 && needed_point_indices_capacity != 0) {
        cmd_buffer->upload_to_buffer(allocator->get_general_buffer(*points_buffer),
                                     0,
                                     segments.points.size() * sizeof(Vec2F),
                                     segments.points.data());

        cmd_buffer->upload_to_buffer(allocator->get_general_buffer(*point_indices_buffer),
                                     0,
                                     segments.indices.size() * sizeof(SegmentIndicesD3D11),
                                     segments.indices.data());
    }
}

void SceneBuffers::upload(SegmentsD3D11 &draw_segments,
                          SegmentsD3D11 &clip_segments,
                          const std::shared_ptr<Pathfinder::Driver> &driver,
                          const std::shared_ptr<CommandBuffer> &cmd_buffer,
                          const std::shared_ptr<GpuMemoryAllocator> &allocator) {
    draw.upload(draw_segments, driver, cmd_buffer, allocator);
    clip.upload(clip_segments, driver, cmd_buffer, allocator);
}

RendererD3D11::RendererD3D11(const std::shared_ptr<Pathfinder::Driver> &driver) : Renderer(driver) {
    allocated_microline_count = INITIAL_ALLOCATED_MICROLINE_COUNT;
    allocated_fill_count = INITIAL_ALLOCATED_FILL_COUNT;

    // Create uniform buffers.
    bin_ub = driver->create_buffer({BufferType::Uniform, 4 * sizeof(int32_t), MemoryProperty::HostVisibleAndCoherent},
                                   "bin uniform buffer");
    bound_ub = driver->create_buffer({BufferType::Uniform, 4 * sizeof(int32_t), MemoryProperty::HostVisibleAndCoherent},
                                     "bound uniform buffer");
    dice_ub0 = driver->create_buffer({BufferType::Uniform, 12 * sizeof(float), MemoryProperty::HostVisibleAndCoherent},
                                     "dice uniform buffer 0");
    dice_ub1 = driver->create_buffer({BufferType::Uniform, 4 * sizeof(int32_t), MemoryProperty::HostVisibleAndCoherent},
                                     "dice uniform buffer 1");
    fill_ub = driver->create_buffer({BufferType::Uniform, 4 * sizeof(int32_t), MemoryProperty::HostVisibleAndCoherent},
                                    "fill uniform buffer");
    propagate_ub =
        driver->create_buffer({BufferType::Uniform, 4 * sizeof(int32_t), MemoryProperty::HostVisibleAndCoherent},
                              "propagate uniform buffer");
    sort_ub = driver->create_buffer({BufferType::Uniform, 4 * sizeof(int32_t), MemoryProperty::HostVisibleAndCoherent},
                                    "sort uniform buffer");
    tile_ub0 = driver->create_buffer({BufferType::Uniform, 8 * sizeof(float), MemoryProperty::HostVisibleAndCoherent},
                                     "tile uniform buffer 0");
    tile_ub1 = driver->create_buffer({BufferType::Uniform, 8 * sizeof(int32_t), MemoryProperty::HostVisibleAndCoherent},
                                     "tile uniform buffer 1");

    // Unlike D3D9, we use RGBA8 instead of RGBA16F for the mask texture.
    mask_texture = driver->create_texture({MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT},
                                          TextureFormat::Rgba8Unorm,
                                          "Mask texture");

    allocator = std::make_shared<GpuMemoryAllocator>(driver);
}

void RendererD3D11::set_up_pipelines() {
    #ifdef PATHFINDER_USE_VULKAN
    const auto dice_source = std::vector<char>(std::begin(dice_comp_spv), std::end(dice_comp_spv));
    const auto bound_source = std::vector<char>(std::begin(bound_comp_spv), std::end(bound_comp_spv));
    const auto bin_source = std::vector<char>(std::begin(bin_comp_spv), std::end(bin_comp_spv));
    const auto propagate_source = std::vector<char>(std::begin(propagate_comp_spv), std::end(propagate_comp_spv));
    const auto fill_source = std::vector<char>(std::begin(fill_comp_spv), std::end(fill_comp_spv));
    const auto sort_source = std::vector<char>(std::begin(sort_comp_spv), std::end(sort_comp_spv));
    const auto tile_source = std::vector<char>(std::begin(tile_comp_spv), std::end(tile_comp_spv));
    #else
    const auto dice_source = std::vector<char>(std::begin(dice_comp), std::end(dice_comp));
    const auto bound_source = std::vector<char>(std::begin(bound_comp), std::end(bound_comp));
    const auto bin_source = std::vector<char>(std::begin(bin_comp), std::end(bin_comp));
    const auto propagate_source = std::vector<char>(std::begin(propagate_comp), std::end(propagate_comp));
    const auto fill_source = std::vector<char>(std::begin(fill_comp), std::end(fill_comp));
    const auto sort_source = std::vector<char>(std::begin(sort_comp), std::end(sort_comp));
    const auto tile_source = std::vector<char>(std::begin(tile_comp), std::end(tile_comp));
    #endif

    dice_descriptor_set = driver->create_descriptor_set();
    dice_descriptor_set->add_or_update({
        Descriptor::storage(0, ShaderStage::Compute),
        Descriptor::storage(1, ShaderStage::Compute),
        Descriptor::storage(2, ShaderStage::Compute),
        Descriptor::storage(3, ShaderStage::Compute),
        Descriptor::storage(4, ShaderStage::Compute),
        Descriptor::uniform(5, ShaderStage::Compute, "bUniform0", dice_ub0),
        Descriptor::uniform(6, ShaderStage::Compute, "bUniform1", dice_ub1),
    });

    bound_descriptor_set = driver->create_descriptor_set();
    bound_descriptor_set->add_or_update({
        Descriptor::storage(0, ShaderStage::Compute),
        Descriptor::storage(1, ShaderStage::Compute),
        Descriptor::uniform(2, ShaderStage::Compute, "bUniform", bound_ub),
    });

    bin_descriptor_set = driver->create_descriptor_set();
    bin_descriptor_set->add_or_update({
        Descriptor::storage(0, ShaderStage::Compute),
        Descriptor::storage(1, ShaderStage::Compute),
        Descriptor::storage(2, ShaderStage::Compute),
        Descriptor::storage(3, ShaderStage::Compute),
        Descriptor::storage(4, ShaderStage::Compute),
        Descriptor::storage(5, ShaderStage::Compute),
        Descriptor::uniform(6, ShaderStage::Compute, "bUniform", bin_ub),
    });

    propagate_descriptor_set = driver->create_descriptor_set();
    propagate_descriptor_set->add_or_update({
        Descriptor::storage(0, ShaderStage::Compute),
        Descriptor::storage(1, ShaderStage::Compute),
        Descriptor::storage(2, ShaderStage::Compute),
        Descriptor::storage(3, ShaderStage::Compute),
        Descriptor::storage(4, ShaderStage::Compute),
        Descriptor::storage(5, ShaderStage::Compute),
        Descriptor::storage(6, ShaderStage::Compute),
        Descriptor::storage(7, ShaderStage::Compute),
        Descriptor::uniform(8, ShaderStage::Compute, "bUniform", propagate_ub),
    });

    sort_descriptor_set = driver->create_descriptor_set();
    sort_descriptor_set->add_or_update({
        Descriptor::storage(0, ShaderStage::Compute),
        Descriptor::storage(1, ShaderStage::Compute),
        Descriptor::storage(2, ShaderStage::Compute),
        Descriptor::uniform(3, ShaderStage::Compute, "bUniform", sort_ub),
    });

    fill_descriptor_set = driver->create_descriptor_set();
    fill_descriptor_set->add_or_update({
        Descriptor::storage(0, ShaderStage::Compute), // Read only.
        Descriptor::storage(1, ShaderStage::Compute), // Read only.
        Descriptor::storage(2, ShaderStage::Compute), // Read only.
        Descriptor::image(3, ShaderStage::Compute, "uDest", mask_texture),
        Descriptor::sampler(4, ShaderStage::Compute, "uAreaLUT", area_lut_texture),
        Descriptor::uniform(5, ShaderStage::Compute, "bUniform", fill_ub),
    });

    tile_descriptor_set = driver->create_descriptor_set();
    tile_descriptor_set->add_or_update({
        Descriptor::storage(0, ShaderStage::Compute),
        Descriptor::storage(1, ShaderStage::Compute),
        Descriptor::sampler(2, ShaderStage::Compute, "uTextureMetadata"),
        Descriptor::sampler(3, ShaderStage::Compute, "uZBuffer"),
        Descriptor::sampler(4, ShaderStage::Compute, "uColorTexture0"),
        Descriptor::sampler(5, ShaderStage::Compute, "uMaskTexture0", mask_texture),
        Descriptor::sampler(6, ShaderStage::Compute, "uGammaLUT", dummy_texture), // Unused binding.
        Descriptor::image(7, ShaderStage::Compute, "uDestImage"),
        Descriptor::uniform(8, ShaderStage::Compute, "bConstantsUniform", constants_ub),
        Descriptor::uniform(9, ShaderStage::Compute, "bUniform0", tile_ub0),
        Descriptor::uniform(10, ShaderStage::Compute, "bUniform1", tile_ub1),
    });

    // These pipelines will be called by order.
    dice_pipeline = driver->create_compute_pipeline(dice_source, dice_descriptor_set, "dice pipeline");     // 1
    bound_pipeline = driver->create_compute_pipeline(bound_source, bound_descriptor_set, "bound pipeline"); // 2
    bin_pipeline = driver->create_compute_pipeline(bin_source, bin_descriptor_set, "bin pipeline");         // 3
    propagate_pipeline =
        driver->create_compute_pipeline(propagate_source, propagate_descriptor_set, "propagate pipeline"); // 4
    fill_pipeline = driver->create_compute_pipeline(fill_source, fill_descriptor_set, "fill pipeline");    // 5
    sort_pipeline = driver->create_compute_pipeline(sort_source, sort_descriptor_set, "sort pipeline");    // 6
    tile_pipeline = driver->create_compute_pipeline(tile_source, tile_descriptor_set, "tile pipeline");    // 7
}

void RendererD3D11::draw(const std::shared_ptr<SceneBuilder> &_scene_builder) {
    auto *scene_builder = static_cast<SceneBuilderD3D11 *>(_scene_builder.get());

    if (scene_builder->built_segments.draw_segments.points.empty()) {
        return;
    }

    // RenderCommand::UploadSceneD3D11
    upload_scene(scene_builder->built_segments.draw_segments, scene_builder->built_segments.clip_segments);

    alpha_tile_count = 0;
    clear_dest_texture = true;

    // Prepare clip tiles.
    {
        auto &prepare_batches = scene_builder->clip_batches_d3d11->prepare_batches;

        for (auto iter = prepare_batches.rbegin(); iter != prepare_batches.rend(); ++iter) {
            if (iter->path_count > 0) {
                prepare_tiles(*iter);
            }
        }
    }

    // Draw tiles.
    for (auto &batch : scene_builder->tile_batches) {
        prepare_and_draw_tiles(batch);
    }

    // Clear all batch info.
    free_tile_batch_buffers();

    allocator->purge_if_needed();
}

std::shared_ptr<Texture> RendererD3D11::get_dest_texture() {
    return dest_texture;
}

void RendererD3D11::set_dest_texture(const std::shared_ptr<Texture> &new_texture) {
    dest_texture = new_texture;
}

void RendererD3D11::upload_scene(SegmentsD3D11 &draw_segments, SegmentsD3D11 &clip_segments) {
    auto cmd_buffer = driver->create_command_buffer("Upload scene");
    scene_buffers.upload(draw_segments, clip_segments, driver, cmd_buffer, allocator);
    cmd_buffer->submit_and_wait();
}

void RendererD3D11::prepare_and_draw_tiles(DrawTileBatchD3D11 &batch) {
    auto tile_batch_id = batch.tile_batch_data.batch_id;

    prepare_tiles(batch.tile_batch_data);

    auto &batch_info = tile_batch_info[tile_batch_id];

    draw_tiles(batch_info.tiles_d3d11_buffer_id,
               batch_info.first_tile_map_buffer_id,
               batch.render_target,
               batch.metadata_texture,
               batch.color_texture);
}

void RendererD3D11::draw_tiles(uint64_t tiles_d3d11_buffer_id,
                               uint64_t first_tile_map_buffer_id,
                               const RenderTarget &render_target,
                               const std::shared_ptr<Texture> &metadata_texture,
                               const std::shared_ptr<Texture> &color_texture) {
    // The framebuffer mentioned here is different from the target viewport.
    // This doesn't change as long as the destination texture's size doesn't change.
    auto framebuffer_tile_size0 = framebuffer_tile_size();

    // Decide render target.
    Vec2I target_size;
    std::shared_ptr<Texture> target_texture;
    int clear_op;
    // If no specific RenderTarget is given, we render to the destination texture.
    if (render_target.framebuffer == nullptr) {
        target_size = dest_texture->get_size();
        target_texture = dest_texture;
        clear_op = clear_dest_texture ? LOAD_ACTION_CLEAR : LOAD_ACTION_LOAD;
        clear_dest_texture = false;
    } else {
        target_size = render_target.framebuffer->get_size();
        target_texture = render_target.framebuffer->get_texture();
        clear_op = LOAD_ACTION_CLEAR;
    }

    auto cmd_buffer = driver->create_command_buffer("Draw tiles");

    Vec2F color_tex_size = color_texture ? color_texture->get_size().to_f32() : Vec2F(0);

    // Update uniform buffers.
    std::array<float, 8> ubo_data0 = {0,
                                      0,
                                      0,
                                      0, // uClearColor
                                      color_tex_size.x,
                                      color_tex_size.y, // uColorTextureSize0
                                      (float)target_size.x,
                                      (float)target_size.y}; // uFramebufferSize

    std::array<int32_t, 5> ubo_data1 = {0,
                                        0, // uZBufferSize
                                        (int32_t)framebuffer_tile_size0.x,
                                        (int32_t)framebuffer_tile_size0.y, // uFramebufferTileSize
                                        clear_op};                         // uLoadAction

    cmd_buffer->upload_to_buffer(tile_ub0, 0, 8 * sizeof(float), ubo_data0.data());
    cmd_buffer->upload_to_buffer(tile_ub1, 0, 5 * sizeof(int32_t), ubo_data1.data());

    // Update descriptor set.
    tile_descriptor_set->add_or_update({
        // Read only.
        Descriptor::storage(0, ShaderStage::Compute, allocator->get_general_buffer(tiles_d3d11_buffer_id)),
        // Read only.
        Descriptor::storage(1, ShaderStage::Compute, allocator->get_general_buffer(first_tile_map_buffer_id)),
        Descriptor::sampler(2, ShaderStage::Compute, "uTextureMetadata", metadata_texture),
        Descriptor::sampler(3, ShaderStage::Compute, "uZBuffer", mask_texture),
        Descriptor::sampler(4, ShaderStage::Compute, "uColorTexture0", color_texture ? color_texture : dummy_texture),
        Descriptor::image(7, ShaderStage::Compute, "uDestImage", target_texture),
    });

    cmd_buffer->sync_descriptor_set(tile_descriptor_set);

    cmd_buffer->begin_compute_pass();

    cmd_buffer->bind_compute_pipeline(tile_pipeline);

    cmd_buffer->bind_descriptor_set(tile_descriptor_set);

    cmd_buffer->dispatch(framebuffer_tile_size0.x, framebuffer_tile_size0.y, 1);

    cmd_buffer->end_compute_pass();

    cmd_buffer->submit_and_wait();
}

Vec2I RendererD3D11::tile_size() const {
    auto temp = dest_texture->get_size() + Vec2I(TILE_WIDTH - 1, TILE_HEIGHT - 1);
    return {temp.x / TILE_WIDTH, temp.y / TILE_HEIGHT};
}

uint64_t RendererD3D11::allocate_z_buffer() {
    // This includes the fill indirect draw params because some drivers limit the number of
    // SSBOs to 8 (#373).
    // Add FILL_INDIRECT_DRAW_PARAMS_SIZE in case tile size is zero.
    auto size = tile_size().area() + FILL_INDIRECT_DRAW_PARAMS_SIZE;
    auto buffer_id = allocator->allocate_general_buffer(size * sizeof(int32_t), "Z buffer");

    return buffer_id;
}

uint64_t RendererD3D11::allocate_first_tile_map() {
    auto size = tile_size().area();
    auto buffer_id = allocator->allocate_general_buffer(size * sizeof(FirstTileD3D11), "First tile map buffer");

    return buffer_id;
}

uint64_t RendererD3D11::allocate_alpha_tile_info(uint32_t index_count) {
    auto buffer_id = allocator->allocate_general_buffer(index_count * sizeof(AlphaTileD3D11), "Alpha tile buffer");

    return buffer_id;
}

PropagateMetadataBufferIDsD3D11 RendererD3D11::upload_propagate_metadata(
    std::vector<PropagateMetadataD3D11> &propagate_metadata,
    std::vector<BackdropInfoD3D11> &backdrops) {
    auto propagate_metadata_storage_id =
        allocator->allocate_general_buffer(propagate_metadata.size() * sizeof(PropagateMetadataD3D11),
                                           "Propagate metadata buffer");

    auto cmd_buffer = driver->create_command_buffer("Upload to propagate metadata buffer");
    cmd_buffer->upload_to_buffer(allocator->get_general_buffer(propagate_metadata_storage_id),
                                 0,
                                 propagate_metadata.size() * sizeof(PropagateMetadataD3D11),
                                 propagate_metadata.data());
    cmd_buffer->submit_and_wait();

    auto backdrops_storage_id =
        allocator->allocate_general_buffer(backdrops.size() * sizeof(BackdropInfoD3D11), "Backdrops buffer");

    return {propagate_metadata_storage_id, backdrops_storage_id};
}

void RendererD3D11::upload_initial_backdrops(uint64_t backdrops_buffer_id, std::vector<BackdropInfoD3D11> &backdrops) {
    auto backdrops_buffer = allocator->get_general_buffer(backdrops_buffer_id);

    auto cmd_buffer = driver->create_command_buffer("Upload initial backdrops");
    cmd_buffer->upload_to_buffer(backdrops_buffer, 0, backdrops.size() * sizeof(BackdropInfoD3D11), backdrops.data());
    cmd_buffer->submit_and_wait();
}

void RendererD3D11::prepare_tiles(TileBatchDataD3D11 &batch) {
    // Upload tiles to GPU or allocate them as appropriate.
    auto tiles_d3d11_buffer_id =
        allocator->allocate_general_buffer(batch.tile_count * sizeof(TileD3D11), "Tiles d3d11 buffer");

    // Fetch and/or allocate clip storage as needed.
    shared_ptr<ClipBufferIDs> clip_buffer_ids;
    if (batch.clipped_path_info) {
        auto clip_batch_id = batch.clipped_path_info->clip_batch_id;

        auto clip_tile_batch_info = tile_batch_info[clip_batch_id];
        auto metadata = clip_tile_batch_info.propagate_metadata_buffer_id;
        auto tiles = clip_tile_batch_info.tiles_d3d11_buffer_id;

        clip_buffer_ids = std::make_shared<ClipBufferIDs>(ClipBufferIDs{metadata, tiles});
    }

    // Allocate a Z-buffer.
    auto z_buffer_id = allocate_z_buffer();

    // Propagate backdrops, bin fills, render fills, and/or perform clipping on GPU if necessary.
    // Allocate space for tile lists.
    auto first_tile_map_buffer_id = allocate_first_tile_map();

    auto propagate_metadata_buffer_ids =
        upload_propagate_metadata(batch.prepare_info.propagate_metadata, batch.prepare_info.backdrops);

    // Dice (flatten) segments into micro-lines. We might have to do this twice if our
    // first attempt runs out of space in the storage buffer.
    std::shared_ptr<MicrolineBufferIDsD3D11> microline_storage{};
    for (int _ = 0; _ < 2; _++) {
        microline_storage = dice_segments(batch.prepare_info.dice_metadata,
                                          batch.segment_count,
                                          batch.path_source,
                                          batch.prepare_info.transform);

        // If the microline buffer has been allocated successfully.
        if (microline_storage != nullptr) {
            break;
        }
    }
    if (microline_storage == nullptr) {
        Logger::error("Ran out of space for microlines when dicing!", "D3D11");
    }

    // Initialize tiles, bin segments. We might have to do this twice if our first
    // attempt runs out of space in the fill buffer. If this is the case, we also
    // need to re-initialize tiles and re-upload backdrops because they would have
    // been modified during the first attempt.
    std::shared_ptr<FillBufferInfoD3D11> fill_buffer_info;
    for (int _ = 0; _ < 2; _++) {
        // Initialize tiles.
        bound(tiles_d3d11_buffer_id, batch.tile_count, batch.prepare_info.tile_path_info);

        // Upload backdrops data.
        upload_initial_backdrops(propagate_metadata_buffer_ids.backdrops, batch.prepare_info.backdrops);

        fill_buffer_info =
            bin_segments(*microline_storage, propagate_metadata_buffer_ids, tiles_d3d11_buffer_id, z_buffer_id);

        // If the fill buffer has been allocated successfully.
        if (fill_buffer_info != nullptr) {
            break;
        }
    }
    if (fill_buffer_info == nullptr) {
        Logger::error("Ran out of space for fills when binning!", "D3D11");
    }

    // Free microline storage as it's not needed anymore.
    allocator->free_general_buffer(microline_storage->buffer_id);

    // TODO(pcwalton): If we run out of space for alpha tile indices, propagate multiple times.

    auto alpha_tiles_buffer_id = allocate_alpha_tile_info(batch.tile_count);

    auto propagate_tiles_info = propagate_tiles(batch.prepare_info.backdrops.size(),
                                                tiles_d3d11_buffer_id,
                                                z_buffer_id,
                                                first_tile_map_buffer_id,
                                                alpha_tiles_buffer_id,
                                                propagate_metadata_buffer_ids,
                                                clip_buffer_ids);

    // Free buffer.
    allocator->free_general_buffer(propagate_metadata_buffer_ids.backdrops);

    draw_fills(*fill_buffer_info, tiles_d3d11_buffer_id, alpha_tiles_buffer_id, propagate_tiles_info);

    // Free buffers.
    allocator->free_general_buffer(fill_buffer_info->fill_vertex_buffer_id);
    allocator->free_general_buffer(alpha_tiles_buffer_id);

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

std::shared_ptr<MicrolineBufferIDsD3D11> RendererD3D11::dice_segments(std::vector<DiceMetadataD3D11> &dice_metadata,
                                                                      uint32_t batch_segment_count,
                                                                      PathSource path_source,
                                                                      Transform2 transform) {
    // Allocate some general buffers.
    auto microline_buffer_id =
        allocator->allocate_general_buffer(allocated_microline_count * sizeof(MicrolineD3D11), "Microline buffer");
    auto dice_metadata_buffer_id =
        allocator->allocate_general_buffer(dice_metadata.size() * sizeof(DiceMetadataD3D11), "Dice metadata buffer");
    auto indirect_draw_params_buffer_id =
        allocator->allocate_general_buffer(FILL_INDIRECT_DRAW_PARAMS_SIZE * sizeof(uint32_t),
                                           "Indirect draw params buffer");

    auto microline_buffer = allocator->get_general_buffer(microline_buffer_id);
    auto dice_metadata_buffer = allocator->get_general_buffer(dice_metadata_buffer_id);
    auto indirect_draw_params_buffer = allocator->get_general_buffer(indirect_draw_params_buffer_id);

    // Get scene source buffers.
    auto &scene_source_buffers = path_source == PathSource::Draw ? scene_buffers.draw : scene_buffers.clip;
    auto points_buffer_id = *scene_source_buffers.points_buffer;
    auto point_indices_buffer_id = *scene_source_buffers.point_indices_buffer;
    auto point_indices_count = scene_source_buffers.point_indices_count;

    uint32_t indirect_compute_params[8] = {0, 0, 0, 0, point_indices_count, 0, 0, 0};

    auto cmd_buffer = driver->create_command_buffer("Dice segments");

    // Upload dice indirect draw params, which will be read later.
    cmd_buffer->upload_to_buffer(indirect_draw_params_buffer,
                                 0,
                                 FILL_INDIRECT_DRAW_PARAMS_SIZE * sizeof(uint32_t),
                                 indirect_compute_params);

    // Upload dice metadata.
    cmd_buffer->upload_to_buffer(dice_metadata_buffer,
                                 0,
                                 dice_metadata.size() * sizeof(DiceMetadataD3D11),
                                 dice_metadata.data());

    // Update uniform buffers.
    // Note that a row of mat2 occupies 4 floats just like a mat4.
    std::array<float, 10> ubo_data0 = {transform.m11(),
                                       transform.m21(),
                                       0,
                                       0,
                                       transform.m12(),
                                       transform.m22(),
                                       0,
                                       0,
                                       transform.get_position().x,
                                       transform.get_position().y};
    cmd_buffer->upload_to_buffer(dice_ub0, 0, 10 * sizeof(float), ubo_data0.data());

    std::array<int32_t, 3> ubo_data1 = {static_cast<int32_t>(dice_metadata.size()),
                                        static_cast<int32_t>(batch_segment_count),
                                        static_cast<int32_t>(allocated_microline_count)};
    cmd_buffer->upload_to_buffer(dice_ub1, 0, 3 * sizeof(int32_t), ubo_data1.data());

    auto points_buffer = allocator->get_general_buffer(points_buffer_id);
    auto point_indices_buffer = allocator->get_general_buffer(point_indices_buffer_id);

    // Bind storage buffers.
    dice_descriptor_set->add_or_update({
        // Read and write.
        Descriptor::storage(0, ShaderStage::Compute, indirect_draw_params_buffer),
        // Read only.
        Descriptor::storage(1, ShaderStage::Compute, dice_metadata_buffer),
        // Read only.
        Descriptor::storage(2, ShaderStage::Compute, points_buffer),
        // Read only.
        Descriptor::storage(3, ShaderStage::Compute, point_indices_buffer),
        // Write only.
        Descriptor::storage(4, ShaderStage::Compute, microline_buffer),
    });

    cmd_buffer->sync_descriptor_set(dice_descriptor_set);

    cmd_buffer->begin_compute_pass();

    cmd_buffer->bind_compute_pipeline(dice_pipeline);

    cmd_buffer->bind_descriptor_set(dice_descriptor_set);

    cmd_buffer->dispatch((batch_segment_count + DICE_WORKGROUP_SIZE - 1) / DICE_WORKGROUP_SIZE, 1, 1);

    cmd_buffer->end_compute_pass();

    cmd_buffer->submit_and_wait();

    // Read indirect draw params back to CPU memory.
    indirect_draw_params_buffer->download_via_mapping(FILL_INDIRECT_DRAW_PARAMS_SIZE * sizeof(uint32_t),
                                                      0,
                                                      indirect_compute_params);

    // Free buffers.
    allocator->free_general_buffer(dice_metadata_buffer_id);
    allocator->free_general_buffer(indirect_draw_params_buffer_id);

    // Fetch microline count from indirect draw params.
    auto microline_count = indirect_compute_params[BIN_INDIRECT_DRAW_PARAMS_MICROLINE_COUNT_INDEX];

    // Allocate more space if not allocated enough.
    if (microline_count > allocated_microline_count) {
        allocated_microline_count = upper_power_of_two(microline_count);
        allocator->free_general_buffer(microline_buffer_id);
        return nullptr;
    }

    auto ids = std::make_shared<MicrolineBufferIDsD3D11>();
    ids->buffer_id = microline_buffer_id;
    ids->count = microline_count;

    return ids;
}

void RendererD3D11::bound(uint64_t tiles_d3d11_buffer_id,
                          uint32_t tile_count,
                          std::vector<TilePathInfoD3D11> &tile_path_info) {
    // This is a staging buffer, which will be freed at the end of this function.
    auto path_info_buffer_id =
        allocator->allocate_general_buffer(tile_path_info.size() * sizeof(TilePathInfoD3D11), "Path info buffer");

    auto cmd_buffer = driver->create_command_buffer("Bound");

    // Upload buffer data.
    auto tile_path_info_buffer = allocator->get_general_buffer(path_info_buffer_id);
    cmd_buffer->upload_to_buffer(tile_path_info_buffer,
                                 0,
                                 tile_path_info.size() * sizeof(TilePathInfoD3D11),
                                 tile_path_info.data());

    // Update uniform buffers.
    std::array<int32_t, 2> ubo_data = {static_cast<int32_t>(tile_path_info.size()), static_cast<int32_t>(tile_count)};
    cmd_buffer->upload_to_buffer(bound_ub, 0, 2 * sizeof(int32_t), ubo_data.data());

    // Update the descriptor set.
    bound_descriptor_set->add_or_update({
        // Read only.
        Descriptor::storage(0, ShaderStage::Compute, tile_path_info_buffer),
        // Write only.
        Descriptor::storage(1, ShaderStage::Compute, allocator->get_general_buffer(tiles_d3d11_buffer_id)),
    });

    cmd_buffer->sync_descriptor_set(bound_descriptor_set);

    cmd_buffer->begin_compute_pass();

    cmd_buffer->bind_compute_pipeline(bound_pipeline);

    cmd_buffer->bind_descriptor_set(bound_descriptor_set);

    cmd_buffer->dispatch((tile_count + BOUND_WORKGROUP_SIZE - 1) / BOUND_WORKGROUP_SIZE, 1, 1);

    cmd_buffer->end_compute_pass();

    cmd_buffer->submit_and_wait();

    allocator->free_general_buffer(path_info_buffer_id);
}

std::shared_ptr<FillBufferInfoD3D11> RendererD3D11::bin_segments(
    MicrolineBufferIDsD3D11 &microline_storage,
    PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids,
    uint64_t tiles_d3d11_buffer_id,
    uint64_t z_buffer_id) {
    // What will be the output of this function.
    auto fill_vertex_buffer_id =
        allocator->allocate_general_buffer(allocated_fill_count * sizeof(Fill), "Fill vertex buffer");

    uint32_t indirect_draw_params[FILL_INDIRECT_DRAW_PARAMS_SIZE] = {6, 0, 0, 0, 0, microline_storage.count, 0, 0};

    auto cmd_buffer = driver->create_command_buffer("Bin segments");

    auto z_buffer = allocator->get_general_buffer(z_buffer_id);

    // Upload Z buffer data.
    {
        // Upload fill indirect draw params to header of the Z-buffer.
        // This is in the Z-buffer, not its own buffer, to work around the 8 SSBO limitation on
        // some drivers (#373).
        cmd_buffer->upload_to_buffer(z_buffer,
                                     0,
                                     FILL_INDIRECT_DRAW_PARAMS_SIZE * sizeof(uint32_t),
                                     indirect_draw_params);

        // Update uniform buffers.
        std::array<int32_t, 2> ubo_data = {(int32_t)microline_storage.count, (int32_t)allocated_fill_count};
        cmd_buffer->upload_to_buffer(bin_ub, 0, 2 * sizeof(int32_t), ubo_data.data());
    }

    // Update the descriptor set.
    bin_descriptor_set->add_or_update({
        // Read only.
        Descriptor::storage(0, ShaderStage::Compute, allocator->get_general_buffer(microline_storage.buffer_id)),
        // Read only.
        Descriptor::storage(1,
                            ShaderStage::Compute,
                            allocator->get_general_buffer(propagate_metadata_buffer_ids.propagate_metadata)),
        // Read and write.
        Descriptor::storage(2, ShaderStage::Compute, z_buffer),
        // Write only.
        Descriptor::storage(3, ShaderStage::Compute, allocator->get_general_buffer(fill_vertex_buffer_id)),
        // Read and write.
        Descriptor::storage(4, ShaderStage::Compute, allocator->get_general_buffer(tiles_d3d11_buffer_id)),
        // Read and write.
        Descriptor::storage(5,
                            ShaderStage::Compute,
                            allocator->get_general_buffer(propagate_metadata_buffer_ids.backdrops)),
    });

    cmd_buffer->sync_descriptor_set(bin_descriptor_set);

    cmd_buffer->begin_compute_pass();

    cmd_buffer->bind_compute_pipeline(bin_pipeline);

    cmd_buffer->bind_descriptor_set(bin_descriptor_set);

    cmd_buffer->dispatch((microline_storage.count + BIN_WORKGROUP_SIZE - 1) / BIN_WORKGROUP_SIZE, 1, 1);

    cmd_buffer->end_compute_pass();

    cmd_buffer->submit_and_wait();

    // Read buffer.
    z_buffer->download_via_mapping(FILL_INDIRECT_DRAW_PARAMS_SIZE * sizeof(uint32_t), 0, indirect_draw_params);

    // Get the actual fill count. Do this after the command buffer is submitted.
    auto needed_fill_count = indirect_draw_params[FILL_INDIRECT_DRAW_PARAMS_INSTANCE_COUNT_INDEX];

    // If we didn't allocate enough space for the needed fills, we need to call this function again.
    if (needed_fill_count > allocated_fill_count) {
        allocated_fill_count = upper_power_of_two(needed_fill_count);
        allocator->free_general_buffer(fill_vertex_buffer_id);
        return nullptr;
    }

    auto fill_buffer_info = std::make_shared<FillBufferInfoD3D11>();
    fill_buffer_info->fill_vertex_buffer_id = fill_vertex_buffer_id;

    return fill_buffer_info;
}

PropagateTilesInfoD3D11 RendererD3D11::propagate_tiles(uint32_t column_count,
                                                       uint64_t tiles_d3d11_buffer_id,
                                                       uint64_t z_buffer_id,
                                                       uint64_t first_tile_map_buffer_id,
                                                       uint64_t alpha_tiles_buffer_id,
                                                       PropagateMetadataBufferIDsD3D11 &propagate_metadata_buffer_ids,
                                                       const shared_ptr<ClipBufferIDs> &clip_buffer_ids) {
    auto tiles_d3d11_buffer = allocator->get_general_buffer(tiles_d3d11_buffer_id);
    auto propagate_metadata_buffer = allocator->get_general_buffer(propagate_metadata_buffer_ids.propagate_metadata);
    auto backdrops_buffer = allocator->get_general_buffer(propagate_metadata_buffer_ids.backdrops);
    auto z_buffer = allocator->get_general_buffer(z_buffer_id);
    auto alpha_tiles_buffer = allocator->get_general_buffer(alpha_tiles_buffer_id);

    auto cmd_buffer = driver->create_command_buffer("Propagate tiles");

    // Upload data to buffers.
    // TODO(pcwalton): Zero out the Z-buffer on GPU?
    auto z_buffer_size = tile_size();
    auto tile_area = z_buffer_size.area();
    auto z_buffer_data = std::vector<int32_t>(tile_area, 0);

    // Fill zeros in the Z buffer. Note the offset for the fill indirect params.
    cmd_buffer->upload_to_buffer(z_buffer,
                                 FILL_INDIRECT_DRAW_PARAMS_SIZE * sizeof(uint32_t),
                                 tile_area * sizeof(int32_t),
                                 z_buffer_data.data());

    // TODO(pcwalton): Initialize the first tiles buffer on GPU?
    auto first_tile_map_buffer = allocator->get_general_buffer(first_tile_map_buffer_id);
    auto first_tile_map = std::vector<FirstTileD3D11>(tile_area, FirstTileD3D11());
    cmd_buffer->upload_to_buffer(first_tile_map_buffer, 0, tile_area * sizeof(FirstTileD3D11), first_tile_map.data());

    // Update uniform buffers.
    auto framebuffer_tile_size0 = framebuffer_tile_size();
    std::array<int32_t, 4> ubo_data = {(int32_t)framebuffer_tile_size0.x,
                                       (int32_t)framebuffer_tile_size0.y,
                                       (int32_t)column_count,
                                       (int32_t)alpha_tile_count};
    cmd_buffer->upload_to_buffer(propagate_ub, 0, 4 * sizeof(int32_t), ubo_data.data());

    // Update the descriptor set.
    {
        propagate_descriptor_set->add_or_update({
            // Read only.
            Descriptor::storage(0, ShaderStage::Compute, propagate_metadata_buffer),
            // Read only.
            Descriptor::storage(2, ShaderStage::Compute, backdrops_buffer),
            // Read and write.
            Descriptor::storage(3, ShaderStage::Compute, tiles_d3d11_buffer),
            // Read and write.
            Descriptor::storage(5, ShaderStage::Compute, z_buffer),
            // Read and write.
            Descriptor::storage(6, ShaderStage::Compute, first_tile_map_buffer),
            // Write only.
            Descriptor::storage(7, ShaderStage::Compute, alpha_tiles_buffer),
        });

        if (clip_buffer_ids) {
            auto clip_metadata_buffer = allocator->get_general_buffer(clip_buffer_ids->metadata);
            auto clip_tile_buffer = allocator->get_general_buffer(clip_buffer_ids->tiles);

            propagate_descriptor_set->add_or_update({
                // Read only.
                Descriptor::storage(1, ShaderStage::Compute, clip_metadata_buffer),
                // Read and write.
                Descriptor::storage(4, ShaderStage::Compute, clip_tile_buffer),
            });
        } else { // Placeholders.
            propagate_descriptor_set->add_or_update({
                Descriptor::storage(1, ShaderStage::Compute, propagate_metadata_buffer),
                Descriptor::storage(4, ShaderStage::Compute, tiles_d3d11_buffer),
            });
        }
    }

    cmd_buffer->sync_descriptor_set(propagate_descriptor_set);

    cmd_buffer->begin_compute_pass();

    cmd_buffer->bind_compute_pipeline(propagate_pipeline);

    cmd_buffer->bind_descriptor_set(propagate_descriptor_set);

    cmd_buffer->dispatch((column_count + PROPAGATE_WORKGROUP_SIZE - 1) / PROPAGATE_WORKGROUP_SIZE, 1, 1);

    cmd_buffer->end_compute_pass();

    uint32_t fill_indirect_draw_params[FILL_INDIRECT_DRAW_PARAMS_SIZE];

    cmd_buffer->submit_and_wait();

    // Read buffer.
    z_buffer->download_via_mapping(FILL_INDIRECT_DRAW_PARAMS_SIZE * sizeof(uint32_t), 0, fill_indirect_draw_params);

    // Do this after the command buffer is submitted.
    auto batch_alpha_tile_count = fill_indirect_draw_params[FILL_INDIRECT_DRAW_PARAMS_ALPHA_TILE_COUNT_INDEX];

    auto alpha_tile_start = alpha_tile_count;
    alpha_tile_count += batch_alpha_tile_count;
    auto alpha_tile_end = alpha_tile_count;

    return {Range(alpha_tile_start, alpha_tile_end)};
}

Vec2I RendererD3D11::framebuffer_tile_size() {
    return pixel_size_to_tile_size(dest_texture->get_size());
}

void RendererD3D11::draw_fills(FillBufferInfoD3D11 &fill_storage_info,
                               uint64_t tiles_d3d11_buffer_id,
                               uint64_t alpha_tiles_buffer_id,
                               PropagateTilesInfoD3D11 &propagate_tiles_info) {
    auto alpha_tile_range = propagate_tiles_info.alpha_tile_range;

    // This setup is a workaround for the annoying 64K limit of compute invocation in OpenGL.
    uint32_t _alpha_tile_count = alpha_tile_range.end - alpha_tile_range.start;

    auto cmd_buffer = driver->create_command_buffer("Draw fills");

    // Update uniform buffer.
    auto framebuffer_tile_size0 = framebuffer_tile_size();
    std::array<int32_t, 2> ubo_data = {static_cast<int32_t>(alpha_tile_range.start),
                                       static_cast<int32_t>(alpha_tile_range.end)};
    cmd_buffer->upload_to_buffer(fill_ub, 0, 2 * sizeof(int32_t), ubo_data.data());

    auto fill_vertex_buffer = allocator->get_general_buffer(fill_storage_info.fill_vertex_buffer_id);
    auto tiles_d3d11_buffer = allocator->get_general_buffer(tiles_d3d11_buffer_id);
    auto alpha_tiles_buffer = allocator->get_general_buffer(alpha_tiles_buffer_id);

    // Update descriptor set.
    fill_descriptor_set->add_or_update({
        // Read only.
        Descriptor::storage(0, ShaderStage::Compute, fill_vertex_buffer),
        // Read only.
        Descriptor::storage(1, ShaderStage::Compute, tiles_d3d11_buffer),
        // Read only.
        Descriptor::storage(2, ShaderStage::Compute, alpha_tiles_buffer),
    });

    cmd_buffer->sync_descriptor_set(fill_descriptor_set);

    cmd_buffer->begin_compute_pass();

    cmd_buffer->bind_compute_pipeline(fill_pipeline);

    cmd_buffer->bind_descriptor_set(fill_descriptor_set);

    cmd_buffer->dispatch(std::min(_alpha_tile_count, 1u << 15u), (_alpha_tile_count + (1 << 15) - 1) >> 15, 1);

    cmd_buffer->end_compute_pass();

    cmd_buffer->submit_and_wait();
}

void RendererD3D11::sort_tiles(uint64_t tiles_d3d11_buffer_id,
                               uint64_t first_tile_map_buffer_id,
                               uint64_t z_buffer_id) {
    auto tiles_d3d11_buffer = allocator->get_general_buffer(tiles_d3d11_buffer_id);
    auto first_tile_map_buffer = allocator->get_general_buffer(first_tile_map_buffer_id);
    auto z_buffer = allocator->get_general_buffer(z_buffer_id);

    auto tile_count = framebuffer_tile_size().area();

    auto cmd_buffer = driver->create_command_buffer("Sort tiles");

    // Update uniform buffer.
    cmd_buffer->upload_to_buffer(sort_ub, 0, sizeof(int32_t), &tile_count);

    // Update the descriptor set.
    sort_descriptor_set->add_or_update({
        // Read and write.
        Descriptor::storage(0, ShaderStage::Compute, tiles_d3d11_buffer),
        // Read and write.
        Descriptor::storage(1, ShaderStage::Compute, first_tile_map_buffer),
        // Read only.
        Descriptor::storage(2, ShaderStage::Compute, z_buffer),
    });

    cmd_buffer->sync_descriptor_set(sort_descriptor_set);

    cmd_buffer->begin_compute_pass();

    cmd_buffer->bind_compute_pipeline(sort_pipeline);

    cmd_buffer->bind_descriptor_set(sort_descriptor_set);

    cmd_buffer->dispatch((tile_count + SORT_WORKGROUP_SIZE - 1) / SORT_WORKGROUP_SIZE, 1, 1);

    cmd_buffer->end_compute_pass();

    cmd_buffer->submit_and_wait();
}

void RendererD3D11::free_tile_batch_buffers() {
    while (true) {
        if (tile_batch_info.empty()) {
            break;
        }

        auto &info = tile_batch_info.back();

        allocator->free_general_buffer(info.z_buffer_id);
        allocator->free_general_buffer(info.tiles_d3d11_buffer_id);
        allocator->free_general_buffer(info.propagate_metadata_buffer_id);
        allocator->free_general_buffer(info.first_tile_map_buffer_id);

        tile_batch_info.pop_back();
    }
}

} // namespace Pathfinder

#endif
