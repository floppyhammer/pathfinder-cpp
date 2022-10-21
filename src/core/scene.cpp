#include "scene.h"

#include "d3d11/scene_builder.h"
#include "d3d9/scene_builder.h"
#include "renderer.h"

namespace Pathfinder {

Scene::Scene(uint32_t p_id, RectF p_view_box) : id(p_id), view_box(p_view_box), palette(Palette(p_id)) {
    // Set up a scene builder.
#ifndef PATHFINDER_USE_D3D11
    scene_builder = std::make_shared<SceneBuilderD3D9>(this);
#else
    scene_builder = std::make_shared<SceneBuilderD3D11>(this);
#endif
}

uint32_t Scene::push_paint(const Paint &paint) {
    auto paint_id = palette.push_paint(paint);

    return paint_id;
}

Paint Scene::get_paint(uint32_t paint_id) const {
    return palette.get_paint(paint_id);
}

uint32_t Scene::push_draw_path(const DrawPath &p_path) {
    // Get the path index in the scene.
    auto draw_path_index = draw_paths.size();

    // Push the path.
    draw_paths.push_back(p_path);

    // Update the scene bounds.
    bounds = bounds.union_rect(p_path.outline.bounds);

    // Update the display list.
    if (!display_list.empty() && display_list.back().type == DisplayItem::Type::DrawPaths) {
        display_list.back().path_range.end = draw_path_index + 1;
    } else {
        DisplayItem item;
        item.type = DisplayItem::Type::DrawPaths;
        item.path_range = Range(draw_path_index, draw_path_index + 1);
        display_list.push_back(item);
    }

    // Need to rebuild the scene.
    is_dirty = true;

    return draw_path_index;
}

uint32_t Scene::push_clip_path(const ClipPath &clip_path) {
    bounds = bounds.union_rect(clip_path.outline.bounds);
    uint32_t clip_path_id = clip_paths.size();
    clip_paths.push_back(clip_path);
    return clip_path_id;
}

void Scene::append_scene(const Scene &p_scene) {
    if (p_scene.draw_paths.empty()) return;

    // Extend paths.
    draw_paths.reserve(draw_paths.size() + p_scene.draw_paths.size());
    draw_paths.insert(std::end(draw_paths), std::begin(p_scene.draw_paths), std::end(p_scene.draw_paths));

    // Combine bounds.
    bounds = bounds.union_rect(p_scene.bounds);

    // Need to rebuild the scene.
    is_dirty = true;
}

RenderTarget Scene::push_render_target(const std::shared_ptr<Driver> &driver, Vec2I render_target_size) {
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

RectF Scene::get_view_box() {
    return view_box;
}

void Scene::set_view_box(const RectF &new_view_box) {
    if (new_view_box == view_box) {
        return;
    }

    view_box = new_view_box;

    // We need rebuild the scene if the view box changes.
    is_dirty = true;
}

void Scene::build(std::shared_ptr<Driver> &driver) {
    if (scene_builder) {
        scene_builder->build(driver);
    }
}

void Scene::build_and_render(std::shared_ptr<Renderer> &renderer) {
    build(renderer->driver);

    renderer->draw(scene_builder);
}

} // namespace Pathfinder
