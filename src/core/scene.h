#ifndef PATHFINDER_SCENE_H
#define PATHFINDER_SCENE_H

#include <limits>
#include <map>
#include <vector>

#include "../gpu/framebuffer.h"
#include "data/data.h"
#include "data/path.h"
#include "paint/palette.h"
#include "renderer.h"
#include "scene_builder.h"

namespace Pathfinder {

/// High-level drawing commands.
struct DisplayItem {
    enum class Type {
        /// Draws paths to the render target on top of the stack.
        DrawPaths,

        /// Pushes a render target onto the top of the stack.
        PushRenderTarget,

        /// Pops a render target from the stack.
        PopRenderTarget,
    } type = Type::DrawPaths;

    // TODO: Make this an ID.
    RenderTarget render_target; // For PushRenderTarget.

    Range path_range; // For DrawPaths.
};

struct SceneEpoch {
    uint64_t hi;
    uint64_t lo;
};

struct LastSceneInfo {
    uint32_t scene_id;
    SceneEpoch scene_epoch;
    std::vector<Range> draw_segment_ranges;
};

class SceneBuilder;

/// The scene of paths to be rendered.
class Scene {
public:
    explicit Scene(uint32_t p_id, RectF p_view_box);

    std::vector<DisplayItem> display_list;

    std::vector<DrawPath> draw_paths;

    std::vector<ClipPath> clip_paths;

    /// Contains paints.
    Palette palette;

    /// A globally-unique identifier for the scene.
    uint32_t id;

    SceneEpoch epoch;

    /// Only rebuild the scene when it is dirty.
    bool is_dirty = true;

    /**
     * Adds a shape to the scene, to be drawn on top of all previously-added shapes.
     * If a render target is on the stack (see `push_render_target()`), the path goes to the
     * render target. Otherwise, it goes to the main output.
     * @param p_path Draw path to add.
     * @return An ID which can later be used to retrieve the path.
     */
    uint32_t push_draw_path(const DrawPath &p_path);

    /// Defines a clip path. Returns an ID that can be used to later clip draw paths.
    uint32_t push_clip_path(const ClipPath &clip_path);

    /// Directs subsequent draw paths to draw to the given render target instead of the output.
    ///
    /// Render targets form a stack. All `push_draw_path()` commands go to the render target at the
    /// top of the stack.
    RenderTarget push_render_target(const std::shared_ptr<Driver> &driver, Vec2<int> render_target_size);

    /// Removes the most-recently-pushed render target from the top of the stack.
    ///
    /// After calling this method, drawing will go to the previous render target. If no render
    /// targets remain on the stack, drawing goes to the main output.
    void pop_render_target();

    /**
     * Add all shapes in a scene to this one.
     * @param p_scene Scene to append.
     */
    void append_scene(const Scene &p_scene);

    /**
     * Defines a new paint, which specifies how paths are to be filled or stroked.
     * @return ID that can be later specified alongside draw paths.
     */
    uint32_t push_paint(const Paint &paint);

    /// Returns the paint with the given ID.
    Paint get_paint(uint32_t paint_id) const;

    /// Returns the view box, which defines the visible portion of the scene.
    RectF get_view_box();

    /// Changes the view box.
    void set_view_box(const RectF &new_view_box);

    /// Build the scene by SceneBuilder.
    void build(std::shared_ptr<Driver> &driver);

    /// A convenience method to build and render the scene.
    void build_and_render(std::shared_ptr<Renderer> &renderer);

private:
    /// Path clipping control.
    /// This can be used to make scrollable elements and clipped text.
    RectF bounds;

    /// Scene-wide clipping control.
    RectF view_box;

    /// Scene builder.
    std::shared_ptr<SceneBuilder> scene_builder;
};

} // namespace Pathfinder

#endif // PATHFINDER_SCENE_H
