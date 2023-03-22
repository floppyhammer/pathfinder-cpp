#ifndef PATHFINDER_SCENE_BUILDER_H
#define PATHFINDER_SCENE_BUILDER_H

#include "../gpu/texture.h"
#include "data/built_path.h"
#include "data/data.h"

namespace Pathfinder {

/// Check if we need a new batch due to color texture change.
bool fixup_batch_for_new_path_if_possible(std::shared_ptr<TileBatchTextureInfo>& batch_color_texture,
                                          const BuiltDrawPath& draw_path);

class Scene;

class Renderer;

// A builder doesn't involve GPU related code.
class SceneBuilder {
public:
    explicit SceneBuilder(Scene* _scene) : scene(_scene) {}

    /// Build everything we need for rendering.
    virtual void build(Renderer* renderer) = 0;

    Scene* get_scene() {
        return scene;
    }

protected:
    // Use a raw pointer to avoid cyclic reference.
    Scene* scene{};
};

} // namespace Pathfinder

#endif // PATHFINDER_SCENE_BUILDER_H
