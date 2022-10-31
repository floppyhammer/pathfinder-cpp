#include "scene.h"

#include "d3d11/scene_builder.h"
#include "d3d9/scene_builder.h"
#include "renderer.h"

using std::vector;

namespace Pathfinder {

Scene::Scene(uint32_t _id, RectF _view_box) : id(_id), view_box(_view_box), palette(Palette(_id)) {
    // Create the scene builder.
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

uint32_t Scene::push_draw_path(const DrawPath &draw_path) {
    // Get the path index in the scene.
    auto draw_path_index = draw_paths.size();

    // Push the path.
    draw_paths.push_back(draw_path);

    // Update the scene bounds.
    bounds = bounds.union_rect(draw_path.outline.bounds);

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

void Scene::push_draw_path_with_index(uint32_t draw_path_id) {
    auto new_path_bounds = draw_paths[draw_path_id].outline.bounds;

    bounds = bounds.union_rect(new_path_bounds);

    auto end_path_id = draw_path_id + 1;

    // Get the last DrawPaths display item.
    if (!display_list.empty() && display_list.back().type == DisplayItem::Type::DrawPaths) {
        auto &range = display_list.back().path_range;

        range.end = end_path_id;
    } else { // If there's none.
        DisplayItem display_item;
        display_item.type = DisplayItem::Type::DrawPaths;
        display_item.path_range = {draw_path_id, end_path_id};

        display_list.push_back(display_item);
    }
}

void Scene::append_scene(const Scene &scene, const Transform2 &transform) {
    if (scene.draw_paths.empty()) {
        return;
    }

    auto merged_palette_info = palette.append_palette(scene.palette);

    // Merge clip paths.
    vector<size_t> clip_path_mapping;
    clip_path_mapping.reserve(scene.clip_paths.size());
    for (auto &clip_path : scene.clip_paths) {
        clip_path_mapping.push_back(clip_paths.size());

        auto new_clip_path = clip_path;
        new_clip_path.outline.transform(transform);

        clip_paths.push_back(new_clip_path);
    }

    // Merge draw paths.
    vector<size_t> draw_path_mapping;
    draw_path_mapping.reserve(scene.draw_paths.size());
    for (auto &draw_path : scene.draw_paths) {
        draw_path_mapping.push_back(draw_paths.size());

        auto new_draw_path = draw_path;
        new_draw_path.paint = merged_palette_info.paint_mapping[draw_path.paint];
        if (draw_path.clip_path) {
            new_draw_path.clip_path = std::make_shared<uint32_t>(clip_path_mapping[*draw_path.clip_path]);
        }

        new_draw_path.outline.transform(transform);

        draw_paths.push_back(new_draw_path);
    }

    // Merge display items.
    for (auto &display_item : scene.display_list) {
        switch (display_item.type) {
            case DisplayItem::Type::PushRenderTarget: {
                auto old_render_target_id = display_item.render_target_id;
                auto new_render_target_id = merged_palette_info.render_target_mapping[old_render_target_id];

                DisplayItem new_display_item;
                new_display_item.type = DisplayItem::Type::PushRenderTarget;
                new_display_item.render_target_id = new_render_target_id;

                display_list.push_back(new_display_item);
            } break;
            case DisplayItem::Type::PopRenderTarget: {
                display_list.push_back(display_item);
            } break;
            case DisplayItem::Type::DrawPaths: {
                auto range = display_item.path_range;

                for (uint32_t old_path_index = range.start; old_path_index < range.end; old_path_index++) {
                    auto old_draw_path_id = draw_path_mapping[old_path_index];
                    push_draw_path_with_index(old_draw_path_id);
                }
            } break;
        }
    }

    // Need to rebuild the scene.
    is_dirty = true;
}

RenderTargetId Scene::push_render_target(const RenderTarget &render_target) {
    auto render_target_id = palette.push_render_target(render_target);

    DisplayItem item{};
    item.type = DisplayItem::Type::PushRenderTarget;
    item.render_target_id = render_target_id;

    display_list.push_back(item);

    return render_target_id;
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
