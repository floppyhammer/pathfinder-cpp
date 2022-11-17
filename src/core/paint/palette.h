#ifndef PATHFINDER_PALETTE_H
#define PATHFINDER_PALETTE_H

#include "paint.h"
#include "unordered_map"

namespace Pathfinder {

/// Metadata texture size.
const int32_t TEXTURE_METADATA_ENTRIES_PER_ROW = 128;
const int32_t TEXTURE_METADATA_TEXTURE_WIDTH = TEXTURE_METADATA_ENTRIES_PER_ROW * 10;
const int32_t TEXTURE_METADATA_TEXTURE_HEIGHT = 65536 / TEXTURE_METADATA_ENTRIES_PER_ROW;

struct MergedPaletteInfo {
    std::map<RenderTargetId, RenderTargetId> render_target_mapping;
    std::map<uint16_t, uint16_t> paint_mapping;
};

/// Stores all paints in a scene.
/// A palette will give two things to a renderer:
/// 1. A metadata texture.
/// 2. A vector of PaintMetadata.
struct Palette {
public:
    std::shared_ptr<Texture> metadata_texture;

    explicit Palette(uint32_t _scene_id);

    /// Push a new paint if not already in cache, and return its ID.
    uint32_t push_paint(const Paint &paint);

    Paint get_paint(uint32_t paint_id) const;

    RenderTargetId push_render_target(const RenderTarget &render_target);

    RenderTarget get_render_target(RenderTargetId id) const;

    /// Important step.
    std::vector<PaintMetadata> build_paint_info(const std::shared_ptr<Driver> &driver);

    MergedPaletteInfo append_palette(const Palette &palette);

private:
    std::vector<Paint> paints;
    std::vector<RenderTarget> render_targets;
    std::map<Paint, uint32_t> cache;
    uint32_t scene_id;

private:
    std::vector<PaintMetadata> assign_paint_locations(const std::shared_ptr<Driver> &driver);

    /// Calculate color texture transforms.
    void calculate_texture_transforms(std::vector<PaintMetadata> &paint_metadata);

    /// Convert PaintMetadata to TextureMetadataEntry, which will be used in a renderer.
    std::vector<TextureMetadataEntry> create_texture_metadata(const std::vector<PaintMetadata> &_paint_metadata);
};

} // namespace Pathfinder

#endif // PATHFINDER_PALETTE_H
