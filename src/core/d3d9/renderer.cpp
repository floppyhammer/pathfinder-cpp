#include "renderer.h"

#include "../../common/global_macros.h"
#include "../../common/io.h"
#include "../../common/math/mat4.h"
#include "../../common/math/vec3.h"
#include "../../common/timestamp.h"
#include "../../gpu/command_encoder.h"
#include "../../gpu/device.h"
#include "../../gpu/window.h"
#include "../paint/palette.h"

#ifdef PATHFINDER_USE_VULKAN
    #include "../../shaders/generated/fill_frag_spv.h"
    #include "../../shaders/generated/fill_vert_spv.h"
    #include "../../shaders/generated/tile_clip_combine_frag_spv.h"
    #include "../../shaders/generated/tile_clip_combine_vert_spv.h"
    #include "../../shaders/generated/tile_clip_copy_frag_spv.h"
    #include "../../shaders/generated/tile_clip_copy_vert_spv.h"
    #include "../../shaders/generated/tile_frag_spv.h"
    #include "../../shaders/generated/tile_vert_spv.h"
#else
    #include "../../shaders/generated/fill_frag.h"
    #include "../../shaders/generated/fill_vert.h"
    #include "../../shaders/generated/tile_clip_combine_frag.h"
    #include "../../shaders/generated/tile_clip_combine_vert.h"
    #include "../../shaders/generated/tile_clip_copy_frag.h"
    #include "../../shaders/generated/tile_clip_copy_vert.h"
    #include "../../shaders/generated/tile_frag.h"
    #include "../../shaders/generated/tile_vert.h"
#endif

#include <array>

namespace Pathfinder {

static uint16_t QUAD_VERTEX_POSITIONS[12] = {0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1};

const size_t FILL_INSTANCE_SIZE = 12;
const size_t CLIP_TILE_INSTANCE_SIZE = 16;

// 65536
const size_t MAX_FILLS_PER_BATCH = 0x10000;

RendererD3D9::RendererD3D9(const std::shared_ptr<Device> &_device, const std::shared_ptr<Queue> &_queue)
    : Renderer(_device, _queue) {
    mask_render_pass_clear =
        device->create_render_pass(TextureFormat::Rgba16Float, AttachmentLoadOp::Clear, "Mask render pass clear");

    mask_render_pass_load =
        device->create_render_pass(TextureFormat::Rgba16Float, AttachmentLoadOp::Load, "Mask render pass load");

    dest_render_pass_clear =
        device->create_render_pass(TextureFormat::Rgba8Unorm, AttachmentLoadOp::Clear, "Dest render pass clear");

    dest_render_pass_load =
        device->create_render_pass(TextureFormat::Rgba8Unorm, AttachmentLoadOp::Load, "Dest render pass load");

    auto quad_vertex_data_size = sizeof(QUAD_VERTEX_POSITIONS);

    // Quad vertex buffer. Shared by fills and tiles drawing.
    quad_vertex_buffer_id = allocator->allocate_buffer(quad_vertex_data_size, BufferType::Vertex, "Quad vertex buffer");

    auto encoder = device->create_command_encoder("Upload quad vertex data");
    encoder->write_buffer(allocator->get_buffer(quad_vertex_buffer_id),
                          0,
                          quad_vertex_data_size,
                          QUAD_VERTEX_POSITIONS);

    queue->submit_and_wait(encoder);
}

void RendererD3D9::set_dest_texture(const std::shared_ptr<Texture> &texture) {
    dest_framebuffer = device->create_framebuffer(dest_render_pass_clear, texture, "Dest framebuffer");
}

std::shared_ptr<Texture> RendererD3D9::get_dest_texture() {
    // In case there's no destination texture set before.
    if (!dest_framebuffer) {
        return nullptr;
    }
    return dest_framebuffer->get_texture();
}

void RendererD3D9::set_up_pipelines() {
    // Fill pipeline.
    {
#ifdef PATHFINDER_USE_VULKAN
        const auto fill_vert_source = std::vector<char>(std::begin(fill_vert_spv), std::end(fill_vert_spv));
        const auto fill_frag_source = std::vector<char>(std::begin(fill_frag_spv), std::end(fill_frag_spv));
#else
        const auto fill_vert_source = std::vector<char>(std::begin(fill_vert), std::end(fill_vert));
        const auto fill_frag_source = std::vector<char>(std::begin(fill_frag), std::end(fill_frag));
#endif

        // Set vertex attributes.
        std::vector<VertexInputAttributeDescription> attribute_descriptions;
        {
            attribute_descriptions.reserve(3);

            // Quad vertex.
            attribute_descriptions.push_back({0, 2, DataType::u16, 2 * sizeof(uint16_t), 0, VertexInputRate::Vertex});

            // Vertex stride for the second vertex buffer.
            uint32_t stride = sizeof(Fill);

            // Attributes in the second buffer.
            attribute_descriptions.push_back({1, 4, DataType::u16, stride, 0, VertexInputRate::Instance});
            attribute_descriptions.push_back(
                {1, 1, DataType::u32, stride, offsetof(Fill, link), VertexInputRate::Instance});
        }

        fill_ub_id = allocator->allocate_buffer(sizeof(FillUniformDx9), BufferType::Uniform, "Fill uniform buffer");

        // Set descriptor set.
        fill_descriptor_set = device->create_descriptor_set();
        fill_descriptor_set->add_or_update({
            Descriptor::uniform(0, ShaderStage::Vertex, "bUniform", allocator->get_buffer(fill_ub_id)),
            Descriptor::sampled(1,
                                ShaderStage::Fragment,
                                "uAreaLUT",
                                allocator->get_texture(area_lut_texture_id),
                                get_default_sampler()),
        });

        fill_pipeline = device->create_render_pipeline(fill_vert_source,
                                                       fill_frag_source,
                                                       attribute_descriptions,
                                                       BlendState::from_equal(),
                                                       fill_descriptor_set,
                                                       mask_render_pass_clear,
                                                       "Fill pipeline");
    }

    // Tile pipeline.
    {
#ifdef PATHFINDER_USE_VULKAN
        const auto tile_vert_source = std::vector<char>(std::begin(tile_vert_spv), std::end(tile_vert_spv));
        const auto tile_frag_source = std::vector<char>(std::begin(tile_frag_spv), std::end(tile_frag_spv));
#else
        const auto tile_vert_source = std::vector<char>(std::begin(tile_vert), std::end(tile_vert));
        const auto tile_frag_source = std::vector<char>(std::begin(tile_frag), std::end(tile_frag));
#endif

        // Set vertex attributes.
        std::vector<VertexInputAttributeDescription> attribute_descriptions;
        {
            attribute_descriptions.reserve(6);

            // Quad vertex.
            attribute_descriptions.push_back({0, 2, DataType::u16, 2 * sizeof(uint16_t), 0, VertexInputRate::Vertex});

            // Vertex stride for the second vertex buffer.
            uint32_t stride = sizeof(TileObjectPrimitive);

            // Attributes in the second buffer.
            attribute_descriptions.push_back({1, 2, DataType::i16, stride, 0, VertexInputRate::Instance});
            attribute_descriptions.push_back(
                {1, 4, DataType::u8, stride, offsetof(TileObjectPrimitive, alpha_tile_id), VertexInputRate::Instance});
            attribute_descriptions.push_back(
                {1, 2, DataType::i8, stride, offsetof(TileObjectPrimitive, ctrl), VertexInputRate::Instance});
            attribute_descriptions.push_back(
                {1, 1, DataType::i32, stride, offsetof(TileObjectPrimitive, path_id), VertexInputRate::Instance});
            attribute_descriptions.push_back(
                {1, 1, DataType::u32, stride, offsetof(TileObjectPrimitive, metadata_id), VertexInputRate::Instance});
        }

        // Create uniform buffer.
        tile_ub_id = allocator->allocate_buffer(sizeof(TileUniformDx9), BufferType::Uniform, "Tile uniform buffer");

        // Set descriptor set.
        tile_descriptor_set = device->create_descriptor_set();
        tile_descriptor_set->add_or_update({
            Descriptor::sampled(0, ShaderStage::Vertex, "uTextureMetadata"),
            Descriptor::sampled(1, ShaderStage::Vertex, "uZBuffer"),
            Descriptor::uniform(2, ShaderStage::VertexAndFragment, "bUniform", allocator->get_buffer(tile_ub_id)),
            Descriptor::sampled(3, ShaderStage::Fragment, "uColorTexture0"),
            Descriptor::sampled(4, ShaderStage::Fragment, "uMaskTexture0"),
            // Unused binding.
            Descriptor::sampled(5,
                                ShaderStage::Fragment,
                                "uDestTexture",
                                allocator->get_texture(dummy_texture_id),
                                get_default_sampler()),
            // Unused binding.
            Descriptor::sampled(6,
                                ShaderStage::Fragment,
                                "uGammaLUT",
                                allocator->get_texture(dummy_texture_id),
                                get_default_sampler()),
        });

        tile_pipeline = device->create_render_pipeline(tile_vert_source,
                                                       tile_frag_source,
                                                       attribute_descriptions,
                                                       BlendState::from_over(),
                                                       tile_descriptor_set,
                                                       dest_render_pass_clear,
                                                       "Tile pipeline");
    }

    create_tile_clip_copy_pipeline();

    create_tile_clip_combine_pipeline();
}

TextureFormat RendererD3D9::mask_texture_format() const {
    return TextureFormat::Rgba16Float;
}

void RendererD3D9::reallocate_alpha_tile_pages_if_necessary() {
    uint32_t alpha_tile_pages_needed = (alpha_tile_count + 0xffff) >> 16;

    if (alpha_tile_pages_needed <= mask_storage.allocated_page_count) {
        return;
    }

    auto new_size = Vec2I(MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT * alpha_tile_pages_needed);
    auto format = mask_texture_format();

    auto mask_framebuffer_id = allocator->allocate_framebuffer(new_size, format, "Mask framebuffer");
    auto mask_framebuffer = allocator->get_framebuffer(mask_framebuffer_id);

    mask_storage = MaskStorage{
        mask_framebuffer_id,
        alpha_tile_pages_needed,
    };
}

void RendererD3D9::create_tile_clip_copy_pipeline() {
#ifdef PATHFINDER_USE_VULKAN
    const auto tile_clip_copy_vert_source =
        std::vector<char>(std::begin(tile_clip_copy_vert_spv), std::end(tile_clip_copy_vert_spv));
    const auto tile_clip_copy_frag_source =
        std::vector<char>(std::begin(tile_clip_copy_frag_spv), std::end(tile_clip_copy_frag_spv));
#else
    const auto tile_clip_copy_vert_source =
        std::vector<char>(std::begin(tile_clip_copy_vert), std::end(tile_clip_copy_vert));
    const auto tile_clip_copy_frag_source =
        std::vector<char>(std::begin(tile_clip_copy_frag), std::end(tile_clip_copy_frag));
#endif

    // Set vertex attributes.
    std::vector<VertexInputAttributeDescription> attribute_descriptions;
    {
        attribute_descriptions.reserve(2);

        attribute_descriptions.push_back({0, 2, DataType::u16, 2 * sizeof(uint16_t), 0, VertexInputRate::Vertex});

        // Set stride based on Clip.
        attribute_descriptions.push_back({1, 1, DataType::i32, sizeof(Clip) / 2, 0, VertexInputRate::Instance});
    }

    // Create descriptor set.
    tile_clip_copy_descriptor_set = device->create_descriptor_set();
    tile_clip_copy_descriptor_set->add_or_update({
        Descriptor::uniform(0, ShaderStage::Vertex, "bUniform", allocator->get_buffer(fill_ub_id)),
        Descriptor::sampled(1, ShaderStage::Fragment, "uSrc", nullptr, get_default_sampler()),
    });

    // We have to disable blend for tile clip copy.
    tile_clip_copy_pipeline = device->create_render_pipeline(tile_clip_copy_vert_source,
                                                             tile_clip_copy_frag_source,
                                                             attribute_descriptions,
                                                             {false},
                                                             tile_clip_copy_descriptor_set,
                                                             mask_render_pass_clear,
                                                             "Tile clip copy pipeline");
}

void RendererD3D9::create_tile_clip_combine_pipeline() {
#ifdef PATHFINDER_USE_VULKAN
    const auto vert_source =
        std::vector<char>(std::begin(tile_clip_combine_vert_spv), std::end(tile_clip_combine_vert_spv));
    const auto frag_source =
        std::vector<char>(std::begin(tile_clip_combine_frag_spv), std::end(tile_clip_combine_frag_spv));
#else
    const auto vert_source = std::vector<char>(std::begin(tile_clip_combine_vert), std::end(tile_clip_combine_vert));
    const auto frag_source = std::vector<char>(std::begin(tile_clip_combine_frag), std::end(tile_clip_combine_frag));
#endif

    // Set vertex attributes.
    std::vector<VertexInputAttributeDescription> attribute_descriptions;
    {
        attribute_descriptions.reserve(5);

        attribute_descriptions.push_back({0, 2, DataType::u16, 2 * sizeof(uint16_t), 0, VertexInputRate::Vertex});

        uint32_t stride = sizeof(Clip);
        attribute_descriptions.push_back({1, 1, DataType::i32, stride, 0, VertexInputRate::Instance});
        attribute_descriptions.push_back(
            {1, 1, DataType::i32, stride, offsetof(Clip, dest_backdrop), VertexInputRate::Instance});
        attribute_descriptions.push_back(
            {1, 1, DataType::i32, stride, offsetof(Clip, src_tile_id), VertexInputRate::Instance});
        attribute_descriptions.push_back(
            {1, 1, DataType::i32, stride, offsetof(Clip, src_backdrop), VertexInputRate::Instance});
    }

    // Create descriptor set.
    tile_clip_combine_descriptor_set = device->create_descriptor_set();
    tile_clip_combine_descriptor_set->add_or_update({
        Descriptor::uniform(0, ShaderStage::Vertex, "bUniform", allocator->get_buffer(fill_ub_id)),
        Descriptor::sampled(1, ShaderStage::Fragment, "uSrc", nullptr, nullptr),
    });

    // We have to disable blend for tile clip combine.
    tile_clip_combine_pipeline = device->create_render_pipeline(vert_source,
                                                                frag_source,
                                                                attribute_descriptions,
                                                                {false},
                                                                tile_clip_combine_descriptor_set,
                                                                mask_render_pass_clear,
                                                                "Tile clip combine pipeline");
}

void RendererD3D9::draw(const std::shared_ptr<SceneBuilder> &_scene_builder) {
    auto *scene_builder = static_cast<SceneBuilderD3D9 *>(_scene_builder.get());

    // We are supposed to draw fills before the builder finishes building.
    // However, it seems not providing much performance boost.
    // So, we just leave it as it is for the sake of simplicity.

    alpha_tile_count = 0;

    for (auto &fill : scene_builder->pending_fills) {
        alpha_tile_count = std::max(alpha_tile_count, fill.link + 1);
    }

    reallocate_alpha_tile_pages_if_necessary();

    // No fills to draw.
    if (!scene_builder->pending_fills.empty()) {
        auto encoder = device->create_command_encoder("Upload & draw fills");

        // Upload fills to buffer.
        auto fill_vertex_buffer_id = upload_fills(scene_builder->pending_fills, encoder);

        // We can do fill drawing as soon as the fill vertex buffer is ready.
        draw_fills(fill_vertex_buffer_id, scene_builder->pending_fills.size(), encoder);

        queue->submit_and_wait(encoder);

        allocator->free_buffer(fill_vertex_buffer_id);
    }

    // Tiles need to be drawn after fill drawing and after tile batches are prepared.
    upload_and_draw_tiles(scene_builder->tile_batches);
}

uint64_t RendererD3D9::upload_fills(const std::vector<Fill> &fills, const std::shared_ptr<CommandEncoder> &encoder) {
    auto byte_size = sizeof(Fill) * fills.size();

    auto fill_vertex_buffer_id = allocator->allocate_buffer(byte_size, BufferType::Vertex, "Fill vertex buffer");

    encoder->write_buffer(allocator->get_buffer(fill_vertex_buffer_id), 0, byte_size, (void *)fills.data());

    return fill_vertex_buffer_id;
}

uint64_t RendererD3D9::upload_z_buffer(const DenseTileMap<uint32_t> &z_buffer_map,
                                       const std::shared_ptr<CommandEncoder> &encoder) {
    // Prepare the Z buffer texture.
    // Its size is always the same as the dst framebuffer size.
    // Its size should depend on the batch's dst framebuffer, but it's easier to cache it this way.
    auto z_buffer_texture_id =
        allocator->allocate_texture(z_buffer_map.rect.size(), TextureFormat::Rgba8Unorm, "Z buffer texture");

    auto z_buffer_texture = allocator->get_texture(z_buffer_texture_id);
    encoder->write_texture(z_buffer_texture, {}, z_buffer_map.data.data());

    return z_buffer_texture_id;
}

uint64_t RendererD3D9::upload_tiles(const std::vector<TileObjectPrimitive> &tiles,
                                    const std::shared_ptr<CommandEncoder> &encoder) {
    auto byte_size = sizeof(TileObjectPrimitive) * tiles.size();

    auto tile_vertex_buffer_id = allocator->allocate_buffer(byte_size, BufferType::Vertex, "Tile vertex buffer");

    encoder->write_buffer(allocator->get_buffer(tile_vertex_buffer_id), 0, byte_size, (void *)tiles.data());

    return tile_vertex_buffer_id;
}

void RendererD3D9::upload_and_draw_tiles(const std::vector<DrawTileBatchD3D9> &tile_batches) {
    // Clear the destination framebuffer for the first time.
    clear_dest_texture = true;

    // One draw call for one batch.
    for (const auto &batch : tile_batches) {
        uint32_t tile_count = batch.tiles.size();

        // No tiles to draw.
        if (tile_count == 0) {
            continue;
        }

        // Different batches will use the same tile vertex buffer, so we need to make sure
        // that a batch is down drawing before processing the next batch.
        auto encoder = device->create_command_encoder("Upload & draw tiles");

        // Apply clip paths.
        if (!batch.clips.empty()) {
            auto clip_buffer_info = upload_clip_tiles(batch.clips, encoder);
            clip_tiles(clip_buffer_info, encoder);
            // Although the command buffer is not submitted yet,
            // it's safe to free it here since the GPU resources are not actually freed.
            allocator->free_buffer(clip_buffer_info.clip_buffer_id);
        }

        auto tile_vertex_buffer_id = upload_tiles(batch.tiles, encoder);

        auto z_buffer_texture_id = upload_z_buffer(batch.z_buffer_data, encoder);

        draw_tiles(tile_vertex_buffer_id,
                   tile_count,
                   batch.render_target_id,
                   batch.color_texture_info,
                   z_buffer_texture_id,
                   encoder);

        queue->submit_and_wait(encoder);

        allocator->free_texture(z_buffer_texture_id);
        allocator->free_buffer(tile_vertex_buffer_id);
    }
}

void RendererD3D9::draw_fills(uint64_t fill_vertex_buffer_id,
                              uint32_t fills_count,
                              const std::shared_ptr<CommandEncoder> &encoder) {
    FillUniformDx9 fill_uniform;
    fill_uniform.tile_size = {TILE_WIDTH, TILE_HEIGHT};
    fill_uniform.framebuffer_size = {MASK_FRAMEBUFFER_WIDTH,
                                     (float)(MASK_FRAMEBUFFER_HEIGHT * mask_storage.allocated_page_count)};

    encoder->write_buffer(allocator->get_buffer(fill_ub_id), 0, sizeof(FillUniformDx9), &fill_uniform);

    encoder->begin_render_pass(mask_render_pass_clear,
                               allocator->get_framebuffer(mask_storage.framebuffer_id),
                               ColorF());

    encoder->bind_render_pipeline(fill_pipeline);

    encoder->bind_vertex_buffers(
        {allocator->get_buffer(quad_vertex_buffer_id), allocator->get_buffer(fill_vertex_buffer_id)});

    encoder->bind_descriptor_set(fill_descriptor_set);

    encoder->draw_instanced(6, fills_count);

    encoder->end_render_pass();
}

// Uploads clip tiles from CPU to GPU.
ClipBufferInfo RendererD3D9::upload_clip_tiles(const std::vector<Clip> &clips,
                                               const std::shared_ptr<CommandEncoder> &encoder) {
    uint32_t clip_count = clips.size();

    auto byte_size = sizeof(Clip) * clip_count;

    auto clip_buffer_id = allocator->allocate_buffer(byte_size, BufferType::Vertex, "Clip buffer");

    encoder->write_buffer(allocator->get_buffer(clip_buffer_id), 0, byte_size, (void *)clips.data());

    return {clip_buffer_id, clip_count};
}

void RendererD3D9::clip_tiles(const ClipBufferInfo &clip_buffer_info, const std::shared_ptr<CommandEncoder> &encoder) {
    // A temporary mask framebuffer for clipping.
    auto temp_mask_framebuffer_id = allocator->allocate_framebuffer(
        Vec2I(MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT * mask_storage.allocated_page_count),
        TextureFormat::Rgba16Float,
        "Temp mask framebuffer");

    auto temp_mask_framebuffer = allocator->get_framebuffer(temp_mask_framebuffer_id);

    auto clip_vertex_buffer = allocator->get_buffer(clip_buffer_info.clip_buffer_id);

    tile_clip_copy_descriptor_set->add_or_update({
        Descriptor::sampled(1,
                            ShaderStage::Fragment,
                            "uSrc",
                            allocator->get_framebuffer(mask_storage.framebuffer_id)->get_texture(),
                            get_default_sampler()),
    });

    // Copy out tiles.
    // TODO(pcwalton): Don't do this on GL4.
    {
        encoder->begin_render_pass(mask_render_pass_clear, temp_mask_framebuffer, ColorF());

        encoder->bind_render_pipeline(tile_clip_copy_pipeline);

        encoder->bind_vertex_buffers({allocator->get_buffer(quad_vertex_buffer_id), clip_vertex_buffer});

        encoder->bind_descriptor_set(tile_clip_copy_descriptor_set);

        // Each clip introduces two instances.
        encoder->draw_instanced(6, clip_buffer_info.clip_count * 2);

        encoder->end_render_pass();
    }

    // Combine clip tiles.
    {
        tile_clip_combine_descriptor_set->add_or_update({
            Descriptor::sampled(1,
                                ShaderStage::Fragment,
                                "uSrc",
                                temp_mask_framebuffer->get_texture(),
                                get_default_sampler()),
        });

        encoder->begin_render_pass(mask_render_pass_load,
                                   allocator->get_framebuffer(mask_storage.framebuffer_id),
                                   ColorF());

        encoder->bind_render_pipeline(tile_clip_combine_pipeline);

        encoder->bind_vertex_buffers({allocator->get_buffer(quad_vertex_buffer_id), clip_vertex_buffer});

        encoder->bind_descriptor_set(tile_clip_combine_descriptor_set);

        encoder->draw_instanced(6, clip_buffer_info.clip_count);

        encoder->end_render_pass();
    }

    allocator->free_framebuffer(temp_mask_framebuffer_id);
}

void RendererD3D9::draw_tiles(uint64_t tile_vertex_buffer_id,
                              uint32_t tiles_count,
                              const std::shared_ptr<RenderTargetId> &render_target_id,
                              const std::shared_ptr<TileBatchTextureInfo> &color_texture_info,
                              uint64_t z_buffer_texture_id,
                              const std::shared_ptr<CommandEncoder> &encoder) {
    std::shared_ptr<Framebuffer> target_framebuffer;

    // If no specific RenderTarget is given, we render to the dst framebuffer.
    if (render_target_id == nullptr) {
        // Check if we should clear the dst framebuffer.
        encoder->begin_render_pass(clear_dest_texture ? dest_render_pass_clear : dest_render_pass_load,
                                   dest_framebuffer,
                                   ColorF());

        target_framebuffer = dest_framebuffer;

        // Disable clear for draw calls after this one.
        clear_dest_texture = false;
    }
    // Otherwise, we render to the given render target.
    else {
        auto render_target = get_render_target(*render_target_id);

        auto framebuffer = render_target.framebuffer;

        // We always clear a render target.
        encoder->begin_render_pass(dest_render_pass_clear, framebuffer, ColorF());

        target_framebuffer = framebuffer;
    }

    Vec2F target_framebuffer_size = target_framebuffer->get_size().to_f32();

    auto z_buffer_texture = allocator->get_texture(z_buffer_texture_id);
    auto color_texture = allocator->get_texture(dummy_texture_id);

    auto default_sampler = get_default_sampler();
    auto color_texture_sampler = default_sampler;

    // Update uniform buffers.
    {
        TileUniformDx9 tile_uniform;
        tile_uniform.tile_size = {TILE_WIDTH, TILE_HEIGHT};
        tile_uniform.texture_metadata_size = {TEXTURE_METADATA_TEXTURE_WIDTH, TEXTURE_METADATA_TEXTURE_HEIGHT};
        tile_uniform.mask_texture_size = {MASK_FRAMEBUFFER_WIDTH,
                                          (float)(MASK_FRAMEBUFFER_HEIGHT * mask_storage.allocated_page_count)};

        // Transform matrix (i.e. the model matrix).
        Mat4 model_mat = Mat4(1.f);
        model_mat = model_mat.translate(Vec3F(-1.f, -1.f, 0.f)); // Move to top-left.
        model_mat = model_mat.scale(Vec3F(2.f / target_framebuffer_size.x, 2.f / target_framebuffer_size.y, 1.f));
        tile_uniform.transform = model_mat;

        tile_uniform.framebuffer_size = target_framebuffer_size.to_f32();
        tile_uniform.z_buffer_size = z_buffer_texture->get_size().to_f32();

        if (color_texture_info) {
            auto color_texture_page = pattern_texture_pages[color_texture_info->page_id];
            if (color_texture_page) {
                color_texture = allocator->get_framebuffer(color_texture_page->framebuffer_id)->get_texture();
                color_texture_sampler = get_or_create_sampler(color_texture_info->sampling_flags);

                if (color_texture == nullptr) {
                    Logger::error("Failed to obtain color texture!", "RendererD3D9");
                    return;
                }
            }
        }

        tile_uniform.color_texture_size = color_texture->get_size().to_f32();

        // We don't need to preserve the data until the upload commands are implemented because
        // these uniform buffers are host-visible/coherent.
        encoder->write_buffer(allocator->get_buffer(tile_ub_id), 0, sizeof(TileUniformDx9), &tile_uniform);
    }

    // Update descriptor set.
    tile_descriptor_set->add_or_update({
        Descriptor::sampled(0,
                            ShaderStage::Vertex,
                            "uTextureMetadata",
                            allocator->get_texture(metadata_texture_id),
                            default_sampler),
        Descriptor::sampled(1, ShaderStage::Vertex, "uZBuffer", z_buffer_texture, default_sampler),
        Descriptor::sampled(3, ShaderStage::Fragment, "uColorTexture0", color_texture, color_texture_sampler),
        Descriptor::sampled(4,
                            ShaderStage::Fragment,
                            "uMaskTexture0",
                            allocator->get_framebuffer(mask_storage.framebuffer_id)->get_texture(),
                            get_default_sampler()),
    });

    encoder->bind_render_pipeline(tile_pipeline);

    encoder->bind_vertex_buffers(
        {allocator->get_buffer(quad_vertex_buffer_id), allocator->get_buffer(tile_vertex_buffer_id)});

    encoder->bind_descriptor_set(tile_descriptor_set);

    encoder->draw_instanced(6, tiles_count);

    encoder->end_render_pass();
}

} // namespace Pathfinder
