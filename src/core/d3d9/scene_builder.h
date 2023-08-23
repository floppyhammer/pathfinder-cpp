#ifndef PATHFINDER_D3D9_SCENE_BUILDER_H
#define PATHFINDER_D3D9_SCENE_BUILDER_H

#include <memory>
#include <mutex>
#include <vector>

#include "../data/built_path.h"
#include "../scene_builder.h"
#include "data/alpha_tile_id.h"
#include "data/draw_tile_batch.h"
#include "data/gpu_data.h"

namespace Pathfinder {

class Renderer;

/// For draw path and clip path.
struct PathBuildParams {
    uint32_t path_id{};
    RectF view_box;
    Scene *scene{};
};

/// For draw path only.
struct DrawPathBuildParams {
    // We have to make an explicit constructor because of the reference members.
    DrawPathBuildParams(PathBuildParams _path_build_params,
                        std::vector<PaintMetadata> &_paint_metadata,
                        std::vector<BuiltPath> &_built_clip_paths)
        : path_build_params(_path_build_params), paint_metadata(_paint_metadata), built_clip_paths(_built_clip_paths) {}

    const PathBuildParams path_build_params;
    const std::vector<PaintMetadata> &paint_metadata;
    const std::vector<BuiltPath> &built_clip_paths;
};

/// Builds a scene into rendering data.
/// Such data only changes when the scene becomes dirty and is rebuilt.
class SceneBuilderD3D9 : public SceneBuilder {
public:
    explicit SceneBuilderD3D9() = default;

    // Data that will be sent to a renderer.
    // ------------------------------------------
    // Fills to draw.
    std::vector<Fill> pending_fills;

    // Tiles to draw.
    std::vector<DrawTileBatchD3D9> tile_batches{};

    // Metadata texture data.
    std::vector<TextureMetadataEntry> metadata;
    // ------------------------------------------

    /// Atomic integer. Used for adding fills.
    std::array<std::atomic<size_t>, ALPHA_TILE_LEVEL_COUNT> next_alpha_tile_indices;

    /// Build everything we need for rendering.
    void build(Scene *_scene, Renderer *renderer) override;

private:
    /// For parallel fill insertion.
    std::mutex fill_write_mutex;

    /**
     * Assign built paths into batches.
     * @param built_paths
     */
    void finish_building(const std::vector<BuiltDrawPath> &built_paths);

    /**
     * Build draw paths into built draw paths.
     */
    std::vector<BuiltDrawPath> build_paths_on_cpu(std::vector<PaintMetadata> &paint_metadata);

    BuiltPath build_clip_path_on_cpu(const PathBuildParams &params);

    /**
     * Run in a thread. Run a tiler on a path.
     * @param path_id Unique ID of the path in the scene.
     * @return A built shape.
     */
    BuiltDrawPath build_draw_path_on_cpu(const DrawPathBuildParams &params);

    /// Build patches for built paths.
    void build_tile_batches(const std::vector<BuiltDrawPath> &built_paths);

    /**
     * Send fills to Renderer::buffered_fills. fill_batch.size() = 124.
     * We only do this synchronously for now.
     * @param fill_batch A fill batch.
     */
    void send_fills(const std::vector<Fill> &fill_batch);
};

} // namespace Pathfinder

#endif // PATHFINDER_D3D9_SCENE_BUILDER_H
