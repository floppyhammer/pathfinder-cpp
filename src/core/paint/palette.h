#ifndef PATHFINDER_PALETTE_H
#define PATHFINDER_PALETTE_H

#include "../../gpu/driver.h"
#include "paint.h"
#include "unordered_map"

using std::map;

namespace Pathfinder {

/// Metadata texture size.
const int32_t TEXTURE_METADATA_ENTRIES_PER_ROW = 128;
const int32_t TEXTURE_METADATA_TEXTURE_WIDTH = TEXTURE_METADATA_ENTRIES_PER_ROW * 10;
const int32_t TEXTURE_METADATA_TEXTURE_HEIGHT = 65536 / TEXTURE_METADATA_ENTRIES_PER_ROW;

struct MergedPaletteInfo {
    map<RenderTargetId, RenderTargetId> render_target_mapping;
    map<uint16_t, uint16_t> paint_mapping;
};

/// Stores all paints in a scene.
/// A palette will give two things to a renderer:
/// 1. A metadata texture.
/// 2. A vector of PaintMetadata.
struct Palette {
public:
    explicit Palette(uint32_t p_scene_id);

    /// Push a new paint if not already in cache, and return its ID.
    uint32_t push_paint(const Paint &paint);

    Paint get_paint(uint32_t paint_id) const;

    RenderTargetId push_render_target(const RenderTarget &render_target);

    RenderTarget get_render_target(RenderTargetId id) const;

    /// Important step.
    std::vector<PaintMetadata> build_paint_info(const std::shared_ptr<Driver> &driver);

    std::shared_ptr<Texture> metadata_texture;

    MergedPaletteInfo append_palette(const Palette &palette) {
        // Merge render targets.
        map<RenderTargetId, RenderTargetId> render_target_mapping;
        for (uint32_t old_render_target_index = 0; old_render_target_index < palette.render_targets.size();
             old_render_target_index++) {
            auto old_render_target = palette.render_targets[old_render_target_index];

            RenderTargetId old_render_target_id = {
                palette.scene_id,
                old_render_target_index,
            };

            auto new_render_target_id = push_render_target(old_render_target);
            render_target_mapping.insert({old_render_target_id, new_render_target_id});
        }

        // Merge paints.
        map<uint16_t, uint16_t> paint_mapping;
        for (uint32_t old_paint_index = 0; old_paint_index < palette.paints.size(); old_paint_index++) {
            auto old_paint_id = old_paint_index;
            auto old_paint = palette.paints[old_paint_index];

            uint32_t new_paint_id;
            if (old_paint.get_overlay()) {
                auto &contents = old_paint.get_overlay()->contents;

                if (contents.type == PaintContents::Type::Pattern) {
                    if (contents.pattern.source.type == PatternSource::Type::RenderTarget) {
                        abort();
                    } else {
                        new_paint_id = push_paint(old_paint);
                    }
                } else {
                    new_paint_id = push_paint(old_paint);
                }
            } else {
                new_paint_id = push_paint(old_paint);
            }

            paint_mapping.insert({old_paint_id, new_paint_id});
        }

        return {render_target_mapping, paint_mapping};
    }

private:
    std::vector<Paint> paints;
    std::vector<RenderTarget> render_targets;
    std::map<Paint, uint32_t> cache;
    uint32_t scene_id;

private:
    std::vector<PaintMetadata> assign_paint_locations(const std::shared_ptr<Driver> &driver);

    /// Calculate color texture transforms.
    void calculate_texture_transforms(std::vector<PaintMetadata> &p_paint_metadata);

    /// Convert PaintMetadata to TextureMetadataEntry, which will be used in a renderer.
    std::vector<TextureMetadataEntry> create_texture_metadata(const std::vector<PaintMetadata> &p_paint_metadata);
};

} // namespace Pathfinder

#endif // PATHFINDER_PALETTE_H
