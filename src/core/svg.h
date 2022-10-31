#ifndef PATHFINDER_SVG_H
#define PATHFINDER_SVG_H

#include "canvas.h"
#include "scene.h"

namespace Pathfinder {

/// Analogy to a SVG image.
class SvgScene {
public:
    SvgScene();

    /**
     * @brief Load a SVG file into the scene.
     * @note We need a copy of the input vector as its content will be modified.
     * @param input SVG file content, a copy by value is needed.
     */
    void load_from_memory(std::vector<char> bytes, Canvas &canvas);

    std::shared_ptr<Scene> get_scene() const;

private:
    std::shared_ptr<Scene> scene;
};

} // namespace Pathfinder

#endif // PATHFINDER_SVG_H
