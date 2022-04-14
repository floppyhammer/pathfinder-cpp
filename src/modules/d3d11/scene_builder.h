//
// Created by floppyhammer on 8/27/2021.
//

#ifndef PATHFINDER_D3D11_SCENE_BUILDER_H
#define PATHFINDER_D3D11_SCENE_BUILDER_H

#include "gpu_data.h"
#include "../d3dx/scene.h"

#include <vector>

#ifdef PATHFINDER_USE_D3D11

namespace Pathfinder {
    struct BuiltSegments {
        SegmentsD3D11 draw_segments;
        SegmentsD3D11 clip_segments;

        std::vector<Range> draw_segment_ranges;
        std::vector<Range> clip_segment_ranges;

        static BuiltSegments from_scene(Scene &scene) {
            BuiltSegments built_segments;
            built_segments.draw_segment_ranges.reserve(scene.draw_paths.size());

            for (const auto &draw_path : scene.draw_paths) {
                auto range = built_segments.draw_segments.add_path(draw_path);
                built_segments.draw_segment_ranges.push_back(range);
            }

            return built_segments;
        }
    };

    class SceneBuilderD3D11 {
    public:
        std::shared_ptr<Scene> scene;

        BuiltSegments built_segments;

        // Sent to renderer to draw tiles.
        std::vector<DrawTileBatchD3D11> tile_batches;

        std::vector<TextureMetadataEntry> metadata;

        /**
         * Build everything we need for rendering.
         */
        void build();

    private:
        void finish_building(LastSceneInfo &last_scene);

        void build_tile_batches(LastSceneInfo &last_scene);
    };
}

#endif

#endif //PATHFINDER_D3D11_SCENE_BUILDER_H
