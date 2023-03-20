#ifndef PATHFINDER_PALETTE_H
#define PATHFINDER_PALETTE_H

#include <unordered_map>

#include "paint.h"

namespace Pathfinder {

/// Metadata texture size.
const int32_t TEXTURE_METADATA_ENTRIES_PER_ROW = 128;
const int32_t TEXTURE_METADATA_TEXTURE_WIDTH = TEXTURE_METADATA_ENTRIES_PER_ROW * 10;
const int32_t TEXTURE_METADATA_TEXTURE_HEIGHT = 65536 / TEXTURE_METADATA_ENTRIES_PER_ROW;

struct MergedPaletteInfo {
    std::map<RenderTargetId, RenderTargetId> render_target_mapping;
    std::map<uint16_t, uint16_t> paint_mapping;
};

// Caches CPU texture images from scene to scene.
struct PaintTextureManager {
    TextureAllocator allocator;
    std::map<uint64_t, TextureLocation> cached_images;
};

class Renderer;

/// Stores all paints in a scene.
/// A palette will give two things to a renderer:
/// 1. A metadata texture.
/// 2. A vector of PaintMetadata.
class Palette {
public:
    explicit Palette(uint32_t _scene_id);

    /// Push a new paint if not already in cache, and return its ID.
    uint32_t push_paint(const Paint &paint);

    Paint get_paint(uint32_t paint_id) const;

    RenderTargetId push_render_target(const RenderTarget &render_target);

    RenderTarget get_render_target(RenderTargetId render_target_id) const;

    /// Important step.
    std::vector<PaintMetadata> build_paint_info(const std::shared_ptr<Driver> &driver, Renderer *renderer);

    /// Append another palette to this append_palette, merging paints and render targets.
    MergedPaletteInfo append_palette(const Palette &palette);

    std::shared_ptr<Texture> get_metadata_texture() const;

private:
    std::vector<Paint> paints;
    std::vector<RenderTarget> render_targets;

    /// Cached paints.
    std::map<Paint, uint32_t> cache;

    // Only for IMAGE pattern source management.
    std::shared_ptr<PaintTextureManager> paint_texture_manager;

    /// Which scene this palette belongs to.
    uint32_t scene_id;

private:
    static void free_transient_locations(PaintTextureManager &texture_manager,
                                  const std::vector<TextureLocation> &transient_paint_locations);

    // Frees images that are cached but not used this frame.
    static void free_unused_images(PaintTextureManager &texture_manager, std::set<uint64_t> used_image_hashes);

    std::vector<TextureLocation> assign_render_target_locations(
        std::vector<TextureLocation> &transient_paint_locations);

    PaintLocationsInfo assign_paint_locations(const std::shared_ptr<Driver> &driver,
                                              const std::vector<TextureLocation> &render_target_metadata,
                                              std::vector<TextureLocation> &transient_paint_locations);

    /// Calculate color texture transforms.
    void calculate_texture_transforms(std::vector<PaintMetadata> &paint_metadata);

    /// Convert PaintMetadata to TextureMetadataEntry, which will be used in a renderer.
    static std::vector<TextureMetadataEntry> create_texture_metadata(const std::vector<PaintMetadata> &paint_metadata);

    /// Allocate GPU textures for the images in the paint texture manager.
    void allocate_textures(Renderer *renderer);
};

} // namespace Pathfinder

#endif // PATHFINDER_PALETTE_H
