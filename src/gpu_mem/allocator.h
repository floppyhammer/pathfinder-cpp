#ifndef PATHFINDER_ALLOCATOR_H
#define PATHFINDER_ALLOCATOR_H

#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../common/math/basic.h"
#include "../gpu/device.h"

namespace Pathfinder {

// Everything above 16 MB is allocated exactly for general buffers.
// This improves general buffer re-usability in a GPU memory allocator.
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
    BufferDescriptor descriptor;
    std::string tag;
};

struct TextureAllocation {
    std::shared_ptr<Texture> texture;
    TextureDescriptor descriptor;
    std::string tag;
};

struct FramebufferAllocation {
    std::shared_ptr<Framebuffer> framebuffer;
    TextureDescriptor descriptor;
    std::string tag;
};

enum class FreeObjectKind {
    Buffer,
    Texture,
    Framebuffer,
    Max,
};

struct FreeObject {
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
    FreeObjectKind kind = FreeObjectKind::Max;
    uint64_t id = std::numeric_limits<uint64_t>::max();

    BufferAllocation buffer_allocation;
    TextureAllocation texture_allocation;
    FramebufferAllocation framebuffer_allocation;
};

/// GPU memory management.
// TODO: update GPU debug marker when reusing free objects.
// Currently, only the tag from the first allocation is set as the debug marker.
class GpuMemoryAllocator {
public:
    explicit GpuMemoryAllocator(const std::shared_ptr<Device>& _device) : device(_device) {}

    uint64_t allocate_buffer(size_t byte_size, BufferType type, const std::string& tag);

    uint64_t allocate_texture(Vec2I size, TextureFormat format, const std::string& tag);

    uint64_t allocate_framebuffer(Vec2I size, TextureFormat format, const std::string& tag);

    std::shared_ptr<Buffer> get_buffer(uint64_t id);

    std::shared_ptr<Texture> get_texture(uint64_t id);

    std::shared_ptr<Framebuffer> get_framebuffer(uint64_t id);

    void free_buffer(uint64_t id);

    void free_texture(uint64_t id);

    void free_framebuffer(uint64_t id);

    void purge_if_needed();

    void print_info();

private:
    std::shared_ptr<Device> device;

    std::unordered_map<uint64_t, BufferAllocation> buffers_in_use;
    std::unordered_map<uint64_t, TextureAllocation> textures_in_use;
    std::unordered_map<uint64_t, FramebufferAllocation> framebuffers_in_use;

    uint64_t next_buffer_id = 0;
    uint64_t next_texture_id = 0;
    uint64_t next_framebuffer_id = 0;

    // Framebuffers are render pass dependent.
    std::unordered_map<TextureFormat, std::shared_ptr<RenderPass>> render_pass_cache;

    std::vector<FreeObject> free_objects;

    // Statistic data.
    size_t bytes_committed = 0; // In-use objects.
    size_t bytes_allocated = 0; // In-use objects + free objects.
};

} // namespace Pathfinder

#endif // PATHFINDER_ALLOCATOR_H
