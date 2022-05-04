#ifndef PATHFINDER_D3D9_SCENE_BUILDER_H
#define PATHFINDER_D3D9_SCENE_BUILDER_H

#include "data/alpha_tile_id.h"
#include "data/gpu_data.h"
#include "data/draw_tile_batch.h"
#include "../d3dx/scene.h"
#include "../d3dx/data/built_path.h"

#include <omp.h>
#include <memory>

namespace Pathfinder {
    class SceneBuilderD3D9 {
    public:
        std::shared_ptr<Scene> scene;

        // Atomic int. Used when adding fills.
        size_t next_alpha_tile_indices[ALPHA_TILE_LEVEL_COUNT] = {0, 0};

        // Send to renderer to draw fills.
        std::vector<Fill> pending_fills;

        // Send to renderer to draw tiles.
        std::vector<DrawTileBatch> tile_batches{};

        // Data used to set up the metadata texture.
        std::vector<TextureMetadataEntry> metadata;

        /**
         * Build everything we need for rendering.
         */
        void build();

    private:
        /**
         * Assign built paths into batches.
         * @param built_paths
         */
        void finish_building(const std::vector<BuiltDrawPath> &built_paths);

        /**
         * Build draw paths into built draw paths.
         */
        std::vector<BuiltDrawPath> build_paths_on_cpu();

        /**
         * Run in a thread. Run a tiler on a outline (path).
         * @param path_id Unique ID of the shape in the scene.
         * @return A built shape.
         */
        BuiltDrawPath build_draw_path_on_cpu(uint32_t path_id, omp_lock_t &write_lock);

        /// Build patches for built paths.
        void build_tile_batches(const std::vector<BuiltDrawPath> &built_paths);

        /**
         * Send fills to Renderer::buffered_fills. fill_batch.size() = 124.
         * We only do this synchronously for now.
         * @param fill_batch A fill batch.
         */
        void send_fills(const std::vector<Fill> &fill_batch);
    };
}

#endif //PATHFINDER_D3D9_SCENE_BUILDER_H
