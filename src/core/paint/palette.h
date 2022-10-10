#ifndef PATHFINDER_PALETTE_H
#define PATHFINDER_PALETTE_H

#include "../../gpu/driver.h"
#include "paint.h"

namespace Pathfinder {

/// Stores all paints in a scene.
struct Palette {
public:
    explicit Palette(uint32_t p_scene_id);

    /// Push a new paint if not already in cache,
    /// and return its ID.
    uint32_t push_paint(const Paint &paint);

    Paint get_paint(uint32_t paint_id) const;

    RenderTarget push_render_target(const std::shared_ptr<Driver> &p_driver, const Vec2<int> &render_target_size);

    std::shared_ptr<Framebuffer> get_render_target(uint32_t render_target_id) const;

    /// Core step.
    std::vector<TextureMetadataEntry> build_paint_info(const std::shared_ptr<Driver> &driver);

private:
    uint32_t scene_id;

    std::vector<Paint> paints;

    // TODO: Rendering related resources should not be here.
    std::vector<std::shared_ptr<Framebuffer>> render_targets;

    std::map<Paint, uint32_t> cache;

private:
    std::vector<PaintMetadata> assign_paint_locations(const std::shared_ptr<Driver> &driver);

    /// Calculate color texture transforms.
    void calculate_texture_transforms(std::vector<PaintMetadata> &p_paint_metadata);

    /// Convert PaintMetadata to TextureMetadataEntry, which will be used in a renderer.
    std::vector<TextureMetadataEntry> create_texture_metadata(const std::vector<PaintMetadata> &p_paint_metadata);
};

} // namespace Pathfinder

#endif // PATHFINDER_PALETTE_H
