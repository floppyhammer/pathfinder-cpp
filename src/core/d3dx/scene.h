#ifndef PATHFINDER_SCENE_H
#define PATHFINDER_SCENE_H

#include "paint.h"
#include "data/path.h"
#include "data/data.h"
#include "../../gpu/gl/framebuffer.h"

#include <vector>
#include <limits>
#include <map>

namespace Pathfinder {
    /// High-level drawing commands.
    struct DisplayItem {
        enum class Type {
            DrawPaths,

            /// Pushes a render target onto the top of the stack.
            PushRenderTarget,

            /// Pops a render target from the stack.
            PopRenderTarget,
        } type = Type::DrawPaths;

        RenderTarget render_target;

        /// Draws paths to the render target on top of the stack.
        Range path_range;
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

    /// The vector scene to be rendered.
    class Scene {
    public:
        explicit Scene(uint32_t p_id, Rect<float> p_view_box);

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
        Rect<float> get_view_box();

        /// Changes the view box.
        void set_view_box(const Rect<float> &new_view_box);

    private:
        /// Bounds of all paths.
        Rect<float> bounds;

        /// Scene clipping box.
        Rect<float> view_box;
    };
}

#endif //PATHFINDER_SCENE_H
