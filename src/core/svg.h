#ifndef PATHFINDER_SVG_H
#define PATHFINDER_SVG_H

#include "canvas.h"
#include "scene.h"

namespace Pathfinder {

/// Analogy to a SVG image.
class SvgScene {
public:
    /**
     * @brief Load SVG string into the scene.
     * @param input SVG file content, a copy by value is needed.
     */
    SvgScene(const std::string& svg, Canvas& canvas);

    std::shared_ptr<Scene> get_scene() const;

private:
    std::shared_ptr<Scene> scene;
};

} // namespace Pathfinder

#endif // PATHFINDER_SVG_H
