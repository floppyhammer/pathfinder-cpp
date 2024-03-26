#include "scene.h"

#include "d3d11/scene_builder.h"
#include "d3d9/scene_builder.h"
#include "renderer.h"

namespace Pathfinder {

SceneEpoch::SceneEpoch(uint64_t _hi, uint64_t _lo) {
    hi = _hi;
    lo = _lo;
}

SceneEpoch SceneEpoch::successor() const {
    if (lo == std::numeric_limits<uint64_t>::max()) {
        return {hi + 1, 0};
    } else {
        return {hi, lo + 1};
    }
}

void SceneEpoch::next() {
    *this = successor();
}

Scene::Scene(uint32_t _id, RectF _view_box) : id(_id), palette(Palette(_id)) {
    set_view_box(_view_box);
}

uint32_t Scene::push_paint(const Paint &paint) {
    auto paint_id = palette.push_paint(paint);
    epoch.next();
    return paint_id;
}

Paint Scene::get_paint(uint32_t paint_id) const {
    return palette.get_paint(paint_id);
}

uint32_t Scene::push_draw_path(const DrawPath &draw_path) {
    auto draw_path_index = draw_paths.size();

    draw_paths.push_back(draw_path);

    push_draw_path_with_index(draw_path_index);

    return draw_path_index;
}

uint32_t Scene::push_clip_path(const ClipPath &clip_path) {
    bounds = bounds.union_rect(clip_path.outline.bounds);
    uint32_t clip_path_id = clip_paths.size();
    clip_paths.push_back(clip_path);
    epoch.next();
    return clip_path_id;
}

void Scene::push_draw_path_with_index(uint32_t draw_path_id) {
    auto new_path_bounds = draw_paths[draw_path_id].outline.bounds;

    bounds = bounds.union_rect(new_path_bounds);

    auto end_path_id = draw_path_id + 1;

    // Get the last DrawPaths display item.
    if (!display_list.empty() && display_list.back().type == DisplayItem::Type::DrawPaths) {
        auto &range = display_list.back().range;

        range.end = end_path_id;
    } else { // If there's none.
        DisplayItem display_item;
        display_item.type = DisplayItem::Type::DrawPaths;
        display_item.range = {draw_path_id, end_path_id};

        display_list.push_back(display_item);
    }

    epoch.next();
}

void Scene::append_scene(const Scene &scene, const Transform2 &transform) {
    if (scene.draw_paths.empty()) {
        return;
    }

    auto merged_palette_info = palette.append_palette(scene.palette, transform);

    // Merge clip paths.
    std::vector<size_t> clip_path_mapping;
    clip_path_mapping.reserve(scene.clip_paths.size());
    for (auto &clip_path : scene.clip_paths) {
        clip_path_mapping.push_back(clip_paths.size());

        auto new_clip_path = clip_path;
        new_clip_path.outline.transform(transform);

        clip_paths.push_back(new_clip_path);
    }

    // Merge draw paths.
    std::vector<size_t> draw_path_mapping;
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
                auto range = display_item.range;

                for (uint32_t old_path_index = range.start; old_path_index < range.end; old_path_index++) {
                    auto new_draw_path_id = draw_path_mapping[old_path_index];
                    push_draw_path_with_index(new_draw_path_id);
                }
            } break;
        }
    }

    // Bump epoch.
    epoch.next();
}

RenderTargetId Scene::push_render_target(const RenderTargetDesc &render_target_desc) {
    auto render_target_id = palette.push_render_target(render_target_desc);

    DisplayItem item{};
    item.type = DisplayItem::Type::PushRenderTarget;
    item.render_target_id = render_target_id;

    display_list.push_back(item);
    epoch.next();

    return render_target_id;
}

void Scene::pop_render_target() {
    DisplayItem item{};
    item.type = DisplayItem::Type::PopRenderTarget;
    display_list.push_back(item);
}

RectF Scene::get_view_box() const {
    return view_box;
}

void Scene::set_view_box(const RectF &new_view_box) {
    view_box = new_view_box;

    // Makes the view box's width a multiple of TILE_WIDTH.
    // Fixes https://github.com/floppyhammer/pathfinder-cpp/issues/21.
    view_box.right = round_up(ceil(new_view_box.right), TILE_WIDTH);
    assert(int(view_box.right) % TILE_WIDTH == 0);

    epoch.next();
}

RectF Scene::get_bounds() const {
    return bounds;
}

void Scene::set_bounds(const RectF &new_bounds) {
    bounds = new_bounds;
    epoch.next();
}

} // namespace Pathfinder
