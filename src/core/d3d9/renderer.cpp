#include "renderer.h"

#include "../../common/global_macros.h"
#include "../../common/io.h"
#include "../../common/math/mat4x4.h"
#include "../../common/math/vec3.h"
#include "../../common/timestamp.h"
#include "../../gpu/command_buffer.h"
#include "../../gpu/driver.h"
#include "../../gpu/platform.h"

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

/// It might not be worth it to cache z buffer textures as they're generally small.
std::shared_ptr<Texture> upload_z_buffer(const std::shared_ptr<Driver> &driver,
                                         const DenseTileMap<uint32_t> &z_buffer_map,
                                         const std::shared_ptr<CommandBuffer> &cmd_buffer) {
    auto z_buffer_texture =
        driver->create_texture(z_buffer_map.rect.size(), TextureFormat::Rgba8Unorm, "Z buffer texture");

    cmd_buffer->upload_to_texture(z_buffer_texture, {}, z_buffer_map.data.data(), TextureLayout::ShaderReadOnly);

    return z_buffer_texture;
}

RendererD3D9::RendererD3D9(const std::shared_ptr<Driver> &p_driver) : Renderer(p_driver) {
    mask_render_pass_clear = driver->create_render_pass(TextureFormat::Rgba16Float,
                                                        AttachmentLoadOp::Clear,
                                                        false,
                                                        "Mask render pass clear");

    mask_render_pass_load =
        driver->create_render_pass(TextureFormat::Rgba16Float, AttachmentLoadOp::Load, false, "Mask render pass load");

    dest_render_pass_clear =
        driver->create_render_pass(TextureFormat::Rgba8Unorm, AttachmentLoadOp::Clear, false, "Dest render pass clear");

    dest_render_pass_load =
        driver->create_render_pass(TextureFormat::Rgba8Unorm, AttachmentLoadOp::Load, false, "Dest render pass load");

    auto mask_texture = driver->create_texture({MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT},
                                               TextureFormat::Rgba16Float,
                                               "Mask texture");
    mask_framebuffer = driver->create_framebuffer(mask_render_pass_clear, mask_texture, "Mask framebuffer");

    // Quad vertex buffer. Shared by fills and tiles drawing.
    quad_vertex_buffer = driver->create_buffer(BufferType::Vertex,
                                               12 * sizeof(uint16_t),
                                               MemoryProperty::DeviceLocal,
                                               "Quad vertex buffer");

    auto cmd_buffer = driver->create_command_buffer(true, "Upload quad vertex data");
    cmd_buffer->upload_to_buffer(quad_vertex_buffer, 0, 12 * sizeof(uint16_t), QUAD_VERTEX_POSITIONS);
    cmd_buffer->submit(driver);
}

void RendererD3D9::set_dest_texture(const std::shared_ptr<Texture> &texture) {
    dest_framebuffer = driver->create_framebuffer(dest_render_pass_clear, texture, "Dest framebuffer");
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

        // Set descriptor set.
        fill_descriptor_set = driver->create_descriptor_set();
        fill_descriptor_set->add_or_update({
            Descriptor::uniform(0, ShaderStage::Vertex, "bConstantSizes", constants_ub),
            Descriptor::sampler(1, ShaderStage::Fragment, "uAreaLUT", area_lut_texture),
        });

        fill_pipeline = driver->create_render_pipeline(fill_vert_source,
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

        // Create uniform buffers.
        tile_transform_ub = driver->create_buffer(BufferType::Uniform,
                                                  16 * sizeof(float),
                                                  MemoryProperty::HostVisibleAndCoherent,
                                                  "Tile transform uniform buffer");
        tile_varying_sizes_ub = driver->create_buffer(BufferType::Uniform,
                                                      8 * sizeof(float),
                                                      MemoryProperty::HostVisibleAndCoherent,
                                                      "Tile varying sizes uniform buffer");

        // Set descriptor set.
        tile_descriptor_set = driver->create_descriptor_set();
        tile_descriptor_set->add_or_update({
            Descriptor::sampler(0, ShaderStage::Vertex, "uTextureMetadata"),
            Descriptor::sampler(1, ShaderStage::Vertex, "uZBuffer"),
            Descriptor::uniform(2, ShaderStage::Vertex, "bTransform", tile_transform_ub),
            Descriptor::uniform(3, ShaderStage::VertexAndFragment, "bVaryingSizes", tile_varying_sizes_ub),
            Descriptor::uniform(4, ShaderStage::VertexAndFragment, "bConstantSizes", constants_ub),
            Descriptor::sampler(5, ShaderStage::Fragment, "uColorTexture0"),
            Descriptor::sampler(6, ShaderStage::Fragment, "uMaskTexture0", mask_framebuffer->get_texture()),
            Descriptor::sampler(7, ShaderStage::Fragment, "uDestTexture"),
            Descriptor::sampler(8, ShaderStage::Fragment, "uGammaLUT"),
        });

        tile_pipeline = driver->create_render_pipeline(tile_vert_source,
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
    tile_clip_copy_descriptor_set = driver->create_descriptor_set();
    tile_clip_copy_descriptor_set->add_or_update({
        Descriptor::uniform(0, ShaderStage::Vertex, "bConstantSizes", constants_ub),
        Descriptor::sampler(1, ShaderStage::Fragment, "uSrc"),
    });

    // We have to disable blend for tile clip copy.
    tile_clip_copy_pipeline = driver->create_render_pipeline(tile_clip_copy_vert_source,
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
    tile_clip_combine_descriptor_set = driver->create_descriptor_set();
    tile_clip_combine_descriptor_set->add_or_update({
        Descriptor::uniform(0, ShaderStage::Vertex, "bConstantSizes", constants_ub),
        Descriptor::sampler(1, ShaderStage::Fragment, "uSrc"),
    });

    // We have to disable blend for tile clip combine.
    tile_clip_combine_pipeline = driver->create_render_pipeline(vert_source,
                                                                frag_source,
                                                                attribute_descriptions,
                                                                {false},
                                                                tile_clip_combine_descriptor_set,
                                                                mask_render_pass_clear,
                                                                "Tile clip combine pipeline");
}

void RendererD3D9::draw(const std::shared_ptr<SceneBuilder> &_scene_builder) {
    auto *scene_builder = static_cast<SceneBuilderD3D9 *>(_scene_builder.get());

    // We are supposed to do this before the builder finishes building.
    // However, it seems not providing much performance boost.
    // So, we just leave it as it is for the sake of simplicity.
    {
        // No fills to draw.
        if (scene_builder->pending_fills.empty()) {
            Logger::warn("No fills to draw!", "RendererD3D9");
        } else {
            auto cmd_buffer = driver->create_command_buffer(true, "Upload & draw fills");

            // Upload fills to buffer.
            upload_fills(scene_builder->pending_fills, cmd_buffer);

            // We can do fill drawing as soon as the fill vertex buffer is ready.
            draw_fills(scene_builder->pending_fills.size(), cmd_buffer);

            cmd_buffer->submit(driver);
        }
    }

    // Tiles need to be drawn after fill drawing and after tile batches are prepared.
    upload_and_draw_tiles(scene_builder->tile_batches);
}

void RendererD3D9::upload_fills(const std::vector<Fill> &fills, const std::shared_ptr<CommandBuffer> &cmd_buffer) {
    auto byte_size = sizeof(Fill) * fills.size();

    // If we need to allocate a new fill vertex buffer.
    if (fill_vertex_buffer == nullptr || byte_size > fill_vertex_buffer->get_size()) {
        fill_vertex_buffer =
            driver->create_buffer(BufferType::Vertex, byte_size, MemoryProperty::DeviceLocal, "Fill vertex buffer");
    }

    cmd_buffer->upload_to_buffer(fill_vertex_buffer, 0, byte_size, (void *)fills.data());
}

void RendererD3D9::upload_tiles(const std::vector<TileObjectPrimitive> &tiles,
                                const std::shared_ptr<CommandBuffer> &cmd_buffer) {
    auto byte_size = sizeof(TileObjectPrimitive) * tiles.size();

    // If we need to allocate a new tile vertex buffer.
    if (tile_vertex_buffer == nullptr || byte_size > tile_vertex_buffer->get_size()) {
        tile_vertex_buffer =
            driver->create_buffer(BufferType::Vertex, byte_size, MemoryProperty::DeviceLocal, "Tile vertex buffer");
    }

    cmd_buffer->upload_to_buffer(tile_vertex_buffer, 0, byte_size, (void *)tiles.data());
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

        // Apply clip paths.
        if (!batch.clips.empty()) {
            auto clip_buffer_info = upload_clip_tiles(batch.clips, driver);
            clip_tiles(clip_buffer_info, driver);
        }

        // Different batches will use the same vertex buffer, so we need to make sure a batch is drawn
        // before processing the next batch.
        auto cmd_buffer = driver->create_command_buffer(true, "Upload & draw tiles");

        upload_tiles(batch.tiles, cmd_buffer);

        auto z_buffer_texture = upload_z_buffer(driver, batch.z_buffer_data, cmd_buffer);

        draw_tiles(tile_count,
                   batch.render_target,
                   batch.metadata_texture,
                   batch.color_texture,
                   z_buffer_texture,
                   cmd_buffer);

        cmd_buffer->submit(driver);
    }
}

void RendererD3D9::draw_fills(uint32_t fills_count, const std::shared_ptr<CommandBuffer> &cmd_buffer) {
    // Mask framebuffer is invalid.
    if (mask_framebuffer == nullptr) {
        Logger::error("RendererD3D9", "Invalid mask framebuffer!");
        return;
    }

    cmd_buffer->sync_descriptor_set(fill_descriptor_set);

    cmd_buffer->begin_render_pass(mask_render_pass_clear, mask_framebuffer, ColorF());

    cmd_buffer->bind_render_pipeline(fill_pipeline);

    cmd_buffer->bind_vertex_buffers({quad_vertex_buffer, fill_vertex_buffer});

    cmd_buffer->bind_descriptor_set(fill_descriptor_set);

    cmd_buffer->draw_instanced(6, fills_count);

    cmd_buffer->end_render_pass();
}

// Uploads clip tiles from CPU to GPU.
ClipBufferInfo RendererD3D9::upload_clip_tiles(const std::vector<Clip> &clips, const std::shared_ptr<Driver> &driver) {
    ClipBufferInfo info;
    info.clip_count = clips.size();

    auto byte_size = sizeof(Clip) * clips.size();

    info.clip_buffer = driver->create_buffer(BufferType::Vertex, byte_size, MemoryProperty::DeviceLocal, "Clip buffer");

    auto cmd_buffer = driver->create_command_buffer(true, "Upload clip tiles");

    cmd_buffer->upload_to_buffer(info.clip_buffer, 0, byte_size, (void *)clips.data());

    cmd_buffer->submit(driver);

    return info;
}

void RendererD3D9::clip_tiles(const ClipBufferInfo &clip_buffer_info, const std::shared_ptr<Driver> &driver) {
    // Allocate temp mask framebuffer.
    // TODO: Cache this.
    auto mask_temp_texture = driver->create_texture({MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT},
                                                    TextureFormat::Rgba16Float,
                                                    "Mask temp texture");
    auto mask_temp_framebuffer =
        driver->create_framebuffer(mask_render_pass_clear, mask_temp_texture, "Mask temp framebuffer");

    auto clip_vertex_buffer = clip_buffer_info.clip_buffer;

    tile_clip_copy_descriptor_set->add_or_update({
        Descriptor::sampler(1, ShaderStage::Fragment, "uSrc", mask_framebuffer->get_texture()),
    });

    // Copy out tiles.
    //
    // TODO(pcwalton): Don't do this on GL4.
    {
        auto cmd_buffer = driver->create_command_buffer(true, "Copy out tiles");

        cmd_buffer->sync_descriptor_set(tile_clip_copy_descriptor_set);

        cmd_buffer->begin_render_pass(mask_render_pass_clear, mask_temp_framebuffer, ColorF());

        cmd_buffer->bind_render_pipeline(tile_clip_copy_pipeline);

        cmd_buffer->bind_vertex_buffers({quad_vertex_buffer, clip_vertex_buffer});

        cmd_buffer->bind_descriptor_set(tile_clip_copy_descriptor_set);

        // Each Clip introduces two instances.
        cmd_buffer->draw_instanced(6, clip_buffer_info.clip_count * 2);

        cmd_buffer->end_render_pass();

        cmd_buffer->submit(driver);
    }

    tile_clip_combine_descriptor_set->add_or_update({
        Descriptor::sampler(1, ShaderStage::Fragment, "uSrc", mask_temp_framebuffer->get_texture()),
    });

    // Combine clip tiles.
    {
        auto cmd_buffer = driver->create_command_buffer(true, "Combine clip tiles");

        cmd_buffer->sync_descriptor_set(tile_clip_combine_descriptor_set);

        cmd_buffer->begin_render_pass(mask_render_pass_load, mask_framebuffer, ColorF());

        cmd_buffer->bind_render_pipeline(tile_clip_combine_pipeline);

        cmd_buffer->bind_vertex_buffers({quad_vertex_buffer, clip_vertex_buffer});

        cmd_buffer->bind_descriptor_set(tile_clip_combine_descriptor_set);

        cmd_buffer->draw_instanced(6, clip_buffer_info.clip_count);

        cmd_buffer->end_render_pass();

        cmd_buffer->submit(driver);
    }
}

void RendererD3D9::draw_tiles(uint32_t tiles_count,
                              const RenderTarget &render_target,
                              const std::shared_ptr<Texture> &metadata_texture,
                              const std::shared_ptr<Texture> &color_texture,
                              const std::shared_ptr<Texture> &z_buffer_texture,
                              const std::shared_ptr<CommandBuffer> &cmd_buffer) {
    std::shared_ptr<Framebuffer> target_framebuffer;

    cmd_buffer->sync_descriptor_set(tile_descriptor_set);

    // If no specific RenderTarget is given.
    if (render_target.framebuffer == nullptr) {
        cmd_buffer->begin_render_pass(clear_dest_texture ? dest_render_pass_clear : dest_render_pass_load,
                                      dest_framebuffer,
                                      ColorF());

        target_framebuffer = dest_framebuffer;

        clear_dest_texture = false;
    } else { // Otherwise, we need to render to that render target.
        cmd_buffer->begin_render_pass(dest_render_pass_clear, render_target.framebuffer, ColorF());

        target_framebuffer = render_target.framebuffer;
    }

    Vec2F target_framebuffer_size = {(float)target_framebuffer->get_width(), (float)target_framebuffer->get_height()};

    // Update uniform buffers.
    {
        // MVP (with only the model matrix).
        auto model_mat = Mat4x4<float>(1.f);
        model_mat = model_mat.translate(Vec3F(-1.f, -1.f, 0.f)); // Move to top-left.
        model_mat = model_mat.scale(Vec3F(2.f / target_framebuffer_size.x, 2.f / target_framebuffer_size.y, 1.f));

        std::array<float, 6> ubo_data = {(float)z_buffer_texture->get_width(),
                                         (float)z_buffer_texture->get_height(),
                                         color_texture ? (float)color_texture->get_width() : 0,
                                         color_texture ? (float)color_texture->get_width() : 0,
                                         target_framebuffer_size.x,
                                         target_framebuffer_size.y};

        // We don't need to preserve the data until the upload commands are implemented because
        // these uniform buffers are host-visible/coherent.
        cmd_buffer->upload_to_buffer(tile_transform_ub, 0, 16 * sizeof(float), &model_mat);
        cmd_buffer->upload_to_buffer(tile_varying_sizes_ub, 0, 6 * sizeof(float), ubo_data.data());
    }

    // Update descriptor set.
    tile_descriptor_set->add_or_update({
        Descriptor::sampler(0, ShaderStage::Vertex, "uTextureMetadata", metadata_texture),
        Descriptor::sampler(1, ShaderStage::Vertex, "uZBuffer", z_buffer_texture),
        Descriptor::sampler(5,
                            ShaderStage::Fragment,
                            "uColorTexture0",
                            color_texture ? color_texture : z_buffer_texture),
        // Unused.
        Descriptor::sampler(7, ShaderStage::Fragment, "uDestTexture", z_buffer_texture),
        // Unused.
        Descriptor::sampler(8, ShaderStage::Fragment, "uGammaLUT", z_buffer_texture),
    });

    cmd_buffer->bind_render_pipeline(tile_pipeline);

    cmd_buffer->bind_vertex_buffers({quad_vertex_buffer, tile_vertex_buffer});

    cmd_buffer->bind_descriptor_set(tile_descriptor_set);

    cmd_buffer->draw_instanced(6, tiles_count);

    cmd_buffer->end_render_pass();
}

std::shared_ptr<Texture> RendererD3D9::get_dest_texture() {
    return dest_framebuffer->get_texture();
}

} // namespace Pathfinder
