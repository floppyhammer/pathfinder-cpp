#ifndef PATHFINDER_SCENE_BUILDER_H
#define PATHFINDER_SCENE_BUILDER_H

#include "scene.h"

namespace Pathfinder {
    class SceneBuilder {
    public:
        // Data that will be sent to a renderer.
        // ------------------------------------------
        // Metadata texture data.
        std::vector<TextureMetadataEntry> metadata;
        // ------------------------------------------

        /// Build everything we need for rendering.
        virtual void build() = 0;

        std::shared_ptr<Scene> get_scene() { return scene; };

        void set_scene(const std::shared_ptr<Scene> &p_scene) { scene = p_scene; };

    protected:
        std::shared_ptr<Scene> scene;
    };
}

#endif //PATHFINDER_SCENE_BUILDER_H
