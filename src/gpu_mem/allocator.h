#ifndef PATHFINDER_ALLOCATOR_H
#define PATHFINDER_ALLOCATOR_H

#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../common/math/basic.h"
#include "../gpu/driver.h"

namespace Pathfinder {

// Everything above 16 MB is allocated exactly for general buffers.
const size_t MAX_BUFFER_SIZE_CLASS = 16 * 1024 * 1024;

// Number of seconds before unused memory is purged.
//
// TODO(pcwalton): jemalloc uses a sigmoidal decay curve here. Consider something similar.
const float DECAY_TIME = 0.250;

// Number of seconds before we can reuse an object buffer.
//
// This helps avoid stalls. This is admittedly a bit of a hack.
const float REUSE_TIME = 0.015;

struct BufferAllocation {
    std::shared_ptr<Buffer> buffer;
    std::string tag;
};

struct TextureAllocation {
    std::shared_ptr<Texture> texture;
    std::string tag;
};

struct FramebufferAllocation {
    std::shared_ptr<Framebuffer> framebuffer;
    std::string tag;
};

enum class FreeObjectKind {
    GeneralBuffer,
    IndexBuffer,
    Texture,
    Framebuffer,
};

struct FreeObject {
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
    FreeObjectKind kind;
    uint64_t id;

    BufferAllocation general_allocation;
    BufferAllocation index_allocation;
    TextureAllocation texture_allocation;
    FramebufferAllocation framebuffer_allocation;
};

class GpuMemoryAllocator {
public:
    GpuMemoryAllocator(const std::shared_ptr<Driver>& _driver) : driver(_driver) {}

    uint64_t allocate_general_buffer(size_t byte_size, const std::string& tag);

    std::shared_ptr<Buffer> get_general_buffer(uint64_t id);

    void free_general_buffer(uint64_t id);

    void purge_if_needed();

private:
    std::shared_ptr<Driver> driver;

    std::unordered_map<uint64_t, BufferAllocation> general_buffers_in_use;
    std::unordered_map<uint64_t, BufferAllocation> index_buffers_in_use;
    std::unordered_map<uint64_t, TextureAllocation> textures_in_use;
    std::unordered_map<uint64_t, FramebufferAllocation> framebuffers_in_use;

    std::vector<FreeObject> free_objects;

    uint64_t next_general_buffer_id;
    uint64_t next_index_buffer_id;
    uint64_t next_texture_id;
    uint64_t next_framebuffer_id;

    // Statistic data.
    size_t bytes_committed = 0;
    size_t bytes_allocated = 0;
};

} // namespace Pathfinder

#endif // PATHFINDER_ALLOCATOR_H
