#ifndef PATHFINDER_SCENE_BUILDER_H
#define PATHFINDER_SCENE_BUILDER_H

#include "../gpu/driver.h"
#include "data/data.h"

namespace Pathfinder {

class Scene;

class SceneBuilder {
public:
    explicit SceneBuilder(Scene* p_scene) : scene(p_scene) {}

    /// Build everything we need for rendering.
    virtual void build(const std::shared_ptr<Driver>& driver) = 0;

    Scene* get_scene() {
        return scene;
    }

    // Metadata texture data, which will be sent to a renderer.
    std::vector<TextureMetadataEntry> metadata{};

protected:
    // Use a raw pointer to avoid cyclic reference.
    Scene* scene{};
};

} // namespace Pathfinder

#endif // PATHFINDER_SCENE_BUILDER_H
