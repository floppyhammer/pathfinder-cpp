#ifndef PATHFINDER_RENDERER_H
#define PATHFINDER_RENDERER_H

#include "scene_builder.h"
#include "data/data.h"
#include "../gpu/driver.h"

#include <cstdint>

namespace Pathfinder {
    /// Mask tile dimension.
    const uint32_t MASK_TILES_ACROSS = 256;
    const uint32_t MASK_TILES_DOWN = 256;

    /// Mask framebuffer.
    // Divide the height by 4 to compress the rows into rgba channels.
    const int32_t MASK_FRAMEBUFFER_WIDTH = TILE_WIDTH * MASK_TILES_ACROSS;
    const int32_t MASK_FRAMEBUFFER_HEIGHT = TILE_HEIGHT / 4 * MASK_TILES_DOWN;

    /// Metadata texture.
    const int32_t TEXTURE_METADATA_ENTRIES_PER_ROW = 128;
    const int32_t TEXTURE_METADATA_TEXTURE_WIDTH = TEXTURE_METADATA_ENTRIES_PER_ROW * 10;
    const int32_t TEXTURE_METADATA_TEXTURE_HEIGHT = 65536 / TEXTURE_METADATA_ENTRIES_PER_ROW;

    void upload_metadata(const std::shared_ptr<Texture> &metadata_texture,
                         const std::vector<TextureMetadataEntry> &metadata,
                         const std::shared_ptr<Driver> &driver);

    /// Base for D3D9 and D3D11 renderers. In most cases, we have only one renderer set up, while having
    /// multiple scenes prepared for rendering.
    class Renderer {
    public:
        explicit Renderer(const std::shared_ptr<Driver> &p_driver);

        /// Set up.
        void set_up(const std::vector<char> &area_lut_input);

        virtual void set_up_pipelines() = 0;

        virtual std::shared_ptr<Texture> get_dest_texture() = 0;

        virtual void set_dest_texture(const std::shared_ptr<Texture> &texture) = 0;

        virtual void draw(const std::shared_ptr<SceneBuilder> &p_scene_builder) = 0;

    protected:
        std::shared_ptr<Driver> driver;

        /// If we should clear the dest framebuffer or texture.
        bool need_to_clear_dest = true;

        /// Pre-defined texture used to draw the mask texture. Shared by D3D9 and D3D10.
        std::shared_ptr<Texture> area_lut_texture;

        /// Texture to store metadata. Shared by D3D9 and D3D10.
        std::shared_ptr<Texture> metadata_texture;

        /// Uniform buffer containing some constants. Shared by D3D9 and D3D10.
        std::shared_ptr<Buffer> fixed_sizes_ub{};
    };
}

#endif //PATHFINDER_RENDERER_H
