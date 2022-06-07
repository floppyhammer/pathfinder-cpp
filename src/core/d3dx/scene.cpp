#include "scene.h"

namespace Pathfinder {
    Scene::Scene(uint32_t p_id, Rect<float> p_view_box) : id(p_id), view_box(p_view_box), palette(Palette(p_id)) {}

    uint32_t Scene::push_paint(const Paint &paint) {
        auto paint_id = palette.push_paint(paint);

        return paint_id;
    }

    Paint Scene::get_paint(uint32_t paint_id) const {
        return palette.get_paint(paint_id);
    }

    uint32_t Scene::push_draw_path(const Shape &p_shape) {
        // Need to rebuild the scene.
        is_dirty = true;

        // Get the shape index in the scene.
        auto draw_path_index = draw_paths.size();

        // Push the shape.
        draw_paths.push_back(p_shape);

        // Update the scene bounds.
        bounds = bounds.union_rect(p_shape.bounds);

        // Update the display list.
        if (!display_list.empty() && display_list.back().type == DisplayItem::Type::DrawPaths) {
            display_list.back().path_range.end = draw_path_index + 1;
        } else {
            DisplayItem item;
            item.type = DisplayItem::Type::DrawPaths;
            item.path_range = Range(draw_path_index, draw_path_index + 1);
            display_list.push_back(item);
        }

        return draw_path_index;
    }

    void Scene::append_scene(const Scene &p_scene) {
        if (p_scene.draw_paths.empty())
            return;

        // Need to rebuild the scene.
        is_dirty = true;

        // Extend shapes.
        draw_paths.reserve(draw_paths.size() + p_scene.draw_paths.size());
        draw_paths.insert(std::end(draw_paths), std::begin(p_scene.draw_paths), std::end(p_scene.draw_paths));

        // Combine bounds.
        bounds = bounds.union_rect(p_scene.bounds);
    }

    void Scene::transform(Vec2<float> p_position, Vec2<float> p_scale, float p_rotation) {
        if (position != p_position) {
            position = p_position;
            is_dirty = true;
        }

        if (scale != p_scale) {
            scale = p_scale;
            is_dirty = true;
        }

        if (rotation != p_rotation) {
            rotation = p_rotation;
            is_dirty = true;
        }
    }

    RenderTarget Scene::push_render_target(const std::shared_ptr<Driver>& driver, Vec2<int> render_target_size) {
        DisplayItem item{};
        item.type = DisplayItem::Type::PushRenderTarget;
        item.render_target = palette.push_render_target(driver, render_target_size);

        display_list.push_back(item);

        return item.render_target;
    }

    void Scene::pop_render_target() {
        DisplayItem item{};
        item.type = DisplayItem::Type::PopRenderTarget;
        display_list.push_back(item);
    }
}
