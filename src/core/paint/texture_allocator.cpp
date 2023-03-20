#include "texture_allocator.h"

#include <cassert>

namespace Pathfinder {

// Invariant: `requested_size` must be a power of two.
RectI TreeNode::allocate(Vec2I this_origin, uint32_t this_size, uint32_t requested_size) {
    if (type == TreeNode::FullLeaf) {
        // No room here.
        return RectI();
    }
    if (this_size < requested_size) {
        // Doesn't fit.
        return RectI();
    }

    // Allocate here or split, as necessary.
    if (type == TreeNode::EmptyLeaf) {
        // Do we have a perfect fit?
        if (this_size == requested_size) {
            type = TreeNode::FullLeaf;
            return RectI(this_origin, Vec2I(this_size, this_size));
        }

        // Split.
        type = TreeNode::Parent;

        for (int i = 0; i < 4; i++) {
            kids[i] = std::make_shared<TreeNode>();
        }
    }

    // Recurse into children.
    switch (type) {
        case TreeNode::Parent: {
            uint32_t kid_size = this_size / 2;

            auto origin = kids[0]->allocate(this_origin, kid_size, requested_size);
            if (origin.is_valid()) {
                return origin;
            }

            origin = kids[1]->allocate(this_origin + Vec2I(kid_size, 0), kid_size, requested_size);
            if (origin.is_valid()) {
                return origin;
            }

            origin = kids[2]->allocate(this_origin + Vec2I(0, kid_size), kid_size, requested_size);
            if (origin.is_valid()) {
                return origin;
            }

            origin = kids[3]->allocate(this_origin + kid_size, kid_size, requested_size);
            if (origin.is_valid()) {
                return origin;
            }

            merge_if_necessary();
            return RectI();
        }
        default: {
            // Unreachable
            abort();
        }
    }
}

void TreeNode::free(Vec2I this_origin, uint32_t this_size, Vec2I requested_origin, uint32_t requested_size) {
    if (this_size <= requested_size) {
        if (this_size == requested_size && this_origin == requested_origin) {
            type = TreeNode::EmptyLeaf;

            for (int i = 0; i < 4; i++) {
                kids[i] = nullptr;
            }
        }
        return;
    }

    uint32_t child_size = this_size / 2;
    auto this_center = this_origin + child_size;

    uint32_t child_index;
    auto child_origin = this_origin;

    if (requested_origin.y < this_center.y) {
        if (requested_origin.x < this_center.x) {
            child_index = 0;
        } else {
            child_index = 1;
            child_origin += Vec2I(child_size, 0);
        }
    } else {
        if (requested_origin.x < this_center.x) {
            child_index = 2;
            child_origin += Vec2I(0, child_size);
        } else {
            child_index = 3;
            child_origin = this_center;
        }
    }

    if (type == TreeNode::Parent) {
        kids[child_index]->free(child_origin, child_size, requested_origin, requested_size);
        merge_if_necessary();
    } else {
        // Unreachable
        abort();
    }
}

void TreeNode::merge_if_necessary() {
    if (type == TreeNode::Parent) {
        // Check if all kids are empty leaves.
        bool res = true;
        for (auto &k : kids) {
            if (k == nullptr) {
                throw std::runtime_error("Parent tree node should not have null kids!");
            }

            if (k->type != TreeNode::EmptyLeaf) {
                res = false;
                break;
            }
        }

        if (res) {
            type = TreeNode::EmptyLeaf;

            for (int i = 0; i < 4; i++) {
                kids[i] = nullptr;
            }
        }
    }
}

RectI TextureAtlasAllocator::allocate(Vec2I requested_size) {
    uint32_t requested_length = upper_power_of_two(std::max(requested_size.x, requested_size.y));
    return root.allocate({}, size, requested_length);
}

void TextureAtlasAllocator::free(RectI rect) {
    uint32_t requested_length = rect.width();
    root.free({}, size, rect.origin(), requested_length);
}

bool TextureAtlasAllocator::is_empty() const {
    if (root.type == TreeNode::EmptyLeaf) {
        return true;
    }
    return false;
}

TextureLocation TextureAllocator::allocate(Vec2I requested_size, AllocationMode mode) {
    // If requested, or if the image is too big, use a separate page.
    if (mode == AllocationMode::OwnPage || requested_size.x > ATLAS_TEXTURE_LENGTH ||
        requested_size.y > ATLAS_TEXTURE_LENGTH) {
        return allocate_image(requested_size);
    }

    // Try to add to each atlas.
    uint32_t first_free_page_index = pages.size();

    for (uint32_t page_index = 0; page_index < pages.size(); page_index++) {
        auto &page = pages[page_index];

        // If the page exists.
        if (page) {
            switch (page->allocator.type) {
                case TexturePageAllocator::Type::Image: {
                    // Skip image pages.
                } break;
                case TexturePageAllocator::Type::Atlas: {
                    auto rect = page->allocator.allocator.allocate(requested_size);

                    // If the insertion to the atlas is successful.
                    if (rect.is_valid()) {
                        return TextureLocation{page_index, rect};
                    }
                } break;
            }
        } else {
            // Found an empty atlas.
            first_free_page_index = std::min(first_free_page_index, page_index);
        }
    }

    // Add a new atlas.

    // TODO(floppyhammer): Should use just first_free_page_index.
    uint32_t page = get_first_free_page_id();

    auto allocator = TextureAtlasAllocator();

    auto rect = allocator.allocate(requested_size);
    if (!rect.is_valid()) {
        throw std::runtime_error("Allocation failed!");
    }

    // Push a new empty page.
    while (page >= pages.size()) {
        pages.push_back(nullptr);
    }

    TexturePageAllocator page_allocator;
    page_allocator.type = TexturePageAllocator::Type::Atlas;
    page_allocator.allocator = allocator;
    pages[page] = std::make_shared<TexturePage>(page_allocator, true);

    return TextureLocation{page, rect};
}

TextureLocation TextureAllocator::allocate_image(Vec2I requested_size) {
    uint32_t page = get_first_free_page_id();

    auto rect = RectI(Vec2I(), requested_size);

    // Push a new empty page.
    while (page >= pages.size()) {
        pages.push_back(nullptr);
    }

    TexturePageAllocator page_allocator;
    page_allocator.type = TexturePageAllocator::Type::Image;
    page_allocator.image_size = rect.size();
    pages[page] = std::make_shared<TexturePage>(page_allocator, true);

    return TextureLocation{page, rect};
}

uint32_t TextureAllocator::get_first_free_page_id() {
    for (uint32_t page_index = 0; page_index < pages.size(); page_index++) {
        auto &page = pages[page_index];
        if (page == nullptr) {
            return page_index;
        }
    }
    return pages.size();
}

void TextureAllocator::free(TextureLocation location) {
    if (pages[location.page]) {
        auto &page_allocator = pages[location.page]->allocator;

        if (page_allocator.type == TexturePageAllocator::Image) {
            assert(location.rect == RectI(Vec2I(), page_allocator.image_size));
        } else {
            auto &atlas_allocator = page_allocator.allocator;
            atlas_allocator.free(location.rect);

            // If there are other textures in the atlas.
            if (!atlas_allocator.is_empty()) {
                // Keep the page around.
                return;
            }
        }
    } else {
        throw std::runtime_error("Texture page is not allocated!");
    }

    // If we got here, free the page.
    // TODO(pcwalton): Actually tell the renderer to free this page!
    pages[location.page] = nullptr;
}

Vec2I TextureAllocator::page_size(uint32_t page_id) {
    if (pages[page_id]) {
        auto &page_allocator = pages[page_id]->allocator;

        if (page_allocator.type == TexturePageAllocator::Atlas) {
            return Vec2I(page_allocator.allocator.size);
        } else {
            return Vec2I(page_allocator.image_size);
        }
    } else {
        throw std::runtime_error("No such texture page!");
    }
}

Vec2F TextureAllocator::page_scale(uint32_t page_id) {
    return Vec2F(1.0) / page_size(page_id).to_f32();
}

bool TextureAllocator::page_is_new(uint32_t page_id) {
    if (pages[page_id]) {
        return pages[page_id]->is_new;
    } else {
        throw std::runtime_error("No such texture page!");
    }
}

void TextureAllocator::mark_all_pages_as_allocated() {
    for (auto &page : pages) {
        if (page) {
            page->is_new = false;
        }
    }
}

TexturePageIter TextureAllocator::page_ids() {
    uint32_t first_index = 0;
    while (first_index < pages.size() && pages[first_index] == nullptr) {
        first_index += 1;
    }
    return TexturePageIter{this, first_index};
}

std::shared_ptr<uint32_t> TexturePageIter::next() {
    std::shared_ptr<uint32_t> next_id;
    if (next_index >= allocator->pages.size()) {
    } else {
        next_id = std::make_shared<uint32_t>(next_index);
    }

    while (true) {
        next_index += 1;
        if (next_index >= allocator->pages.size() || allocator->pages[next_index] != nullptr) {
            break;
        }
    }
    return next_id;
}

bool prop_allocation_and_freeing_work(uint32_t length, std::vector<Vec2I> sizes) {
    length = std::max(upper_power_of_two(length), (unsigned long)1);

    auto allocator = TextureAtlasAllocator::with_length(length);
    std::vector<RectI> locations;

    for (auto &size : sizes) {
        size.x = clamp(size.x, 1, (int32_t)length);
        size.y = clamp(size.y, 1, (int32_t)length);

        auto rect = allocator.allocate(size);
        if (rect.is_valid()) {
            locations.push_back(rect);
        }
    }

    for (auto &location : locations) {
        allocator.free(location);
    }

    assert(allocator.is_empty());

    return true;
}

} // namespace Pathfinder
