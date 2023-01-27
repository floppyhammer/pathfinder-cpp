#ifndef PATHFINDER_TEXTURE_ALLOCATOR_H
#define PATHFINDER_TEXTURE_ALLOCATOR_H

#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../common/math/basic.h"
#include "../common/math/rect.h"
#include "../common/math/vec2.h"
#include "../gpu/driver.h"

namespace Pathfinder {

const uint32_t ATLAS_TEXTURE_LENGTH = 1024;

struct TreeNode {
    enum Type {
        EmptyLeaf,
        FullLeaf,
        Parent,
    } type = EmptyLeaf;

    // Option: Parent.
    /// Top left, top right, bottom left, and bottom right, in that order.
    std::shared_ptr<TreeNode> kids[4]{};

    TreeNode() = default;

    // Invariant: `requested_size` must be a power of two.
    RectI allocate(Vec2I this_origin, uint32_t this_size, uint32_t requested_size);

    void free(Vec2I this_origin, uint32_t this_size, Vec2I requested_origin, uint32_t requested_size);

    void merge_if_necessary();
};

struct TextureAtlasAllocator {
    TreeNode root;

    /// Can be overriden by a given length.
    uint32_t size = ATLAS_TEXTURE_LENGTH;

    /// Create allocator with custom length.
    static TextureAtlasAllocator with_length(uint32_t length) {
        TreeNode node;
        node.type = TreeNode::EmptyLeaf;
        return TextureAtlasAllocator{node, length};
    }

    RectI allocate(Vec2I requested_size);

    void free(RectI rect);

    bool is_empty();
};

struct TexturePageAllocator {
    enum Type {
        Atlas,
        Image,
    } type;

    // Option 1
    /// An atlas allocated with our quadtree allocator.
    TextureAtlasAllocator allocator;

    // Option 2
    /// A single image.
    Vec2I image_size;
};

struct TexturePage {
    TexturePageAllocator allocator;
    bool is_new;

    /// Actual GPU resource.
    std::shared_ptr<Texture> texture;

    TexturePage(TexturePageAllocator _allocator, bool _is_new, const std::shared_ptr<Driver>& driver);
};

enum AllocationMode {
    Atlas,
    OwnPage,
};

struct TextureLocation {
    /// Which texture.
    uint32_t page{};
    /// Region in the texture.
    RectI rect{};
};

struct TexturePageIter;

struct TextureAllocator {
    friend struct TexturePageIter;

public:
    explicit TextureAllocator(std::shared_ptr<Driver> _driver) : driver(std::move(_driver)){};

private:
    std::shared_ptr<Driver> driver;

    std::vector<std::shared_ptr<TexturePage>> pages;

    TextureLocation allocate(Vec2I requested_size, AllocationMode mode);

    TextureLocation allocate_image(Vec2I requested_size);

    uint32_t get_first_free_page_id();

    void free(TextureLocation location);

    Vec2I page_size(uint32_t page_id);

    Vec2F page_scale(uint32_t page_id);

    bool page_is_new(uint32_t page_id);

    void mark_all_pages_as_allocated();

    TexturePageIter page_ids();
};

struct TexturePageIter {
    TextureAllocator* allocator{};
    size_t next_index{};

    std::shared_ptr<uint32_t> next();
};

/// Test.
bool prop_allocation_and_freeing_work(uint32_t length, std::vector<Vec2I> sizes);

} // namespace Pathfinder

#endif // PATHFINDER_TEXTURE_ALLOCATOR_H
