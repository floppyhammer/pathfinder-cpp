#pragma once

#include <array>
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

// Number of seconds before unused memory is purged from idle_pool.
const float DECAY_TIME = 2.0;

// Maximum number of frames that can be in flight on the GPU.
// We assume GPU won't lag behind more than this many frames.
const int MAX_FRAMES_IN_FLIGHT = 3;

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
};

struct FrameBucket {
    std::vector<FreeObject> objects;
};

/// GPU memory management.
class GpuMemoryAllocator {
public:
    explicit GpuMemoryAllocator(const std::shared_ptr<Device>& _device) : device(_device) {}

    uint64_t allocate_buffer(size_t byte_size, BufferType type, const std::string& tag);

    uint64_t allocate_texture(Vec2I size, TextureFormat format, const std::string& tag);

    std::shared_ptr<Buffer> get_buffer(uint64_t id);

    std::shared_ptr<Texture> get_texture(uint64_t id);

    void free_buffer(uint64_t id);

    void free_texture(uint64_t id);

    /// Notify the allocator that a new frame has started.
    /// This reclaims resources from several frames ago.
    void begin_frame();

    void purge_if_needed();

    void print_info();

private:
    std::shared_ptr<Device> device;

    std::unordered_map<uint64_t, BufferAllocation> active_buffers;
    std::unordered_map<uint64_t, TextureAllocation> active_textures;

    uint64_t next_buffer_id = 0;
    uint64_t next_texture_id = 0;

    // Resources that are confirmed to be safe for reuse (at least MAX_FRAMES_IN_FLIGHT old).
    std::vector<FreeObject> idle_pool;

    // Resources organized by their free-frame index.
    std::array<FrameBucket, MAX_FRAMES_IN_FLIGHT> pending_buckets;

    uint32_t current_frame_index = 0;

    // Statistic data.

    /// Total bytes currently held by user logic (Active state).
    size_t bytes_committed = 0;

    /// Total GPU memory allocated on the device (Active + Pending + Idle states).
    size_t bytes_allocated = 0;
};

} // namespace Pathfinder
