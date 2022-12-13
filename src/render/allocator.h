#ifndef PATHFINDER_ALLOCATOR_H
#define PATHFINDER_ALLOCATOR_H

#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../common/math/basic.h"
#include "../common/math/rect.h"
#include "../common/math/vec2.h"
#include "../gpu/driver.h"

namespace Pathfinder {

// Everything above 16 MB is allocated exactly.
// const size_t MAX_BUFFER_SIZE_CLASS = 16 * 1024 * 1024;

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

enum FreeObjectKind {
    GeneralBuffer,
    IndexBuffer,
    Texture,
    Framebuffer,
};

struct FreeObject {
    FreeObjectKind kind;
    uint64_t id;
};

class GpuMemoryAllocator {
public:
    GpuMemoryAllocator(std::shared_ptr<Driver> _driver) : driver(_driver) {}

private:
    std::shared_ptr<Driver> driver;

    std::unordered_map<uint64_t, std::shared_ptr<Buffer>> general_buffers_in_use;
    std::unordered_map<uint64_t, std::shared_ptr<Buffer>> index_buffers_in_use;
    std::unordered_map<uint64_t, std::shared_ptr<Texture>> textures_in_use;
    std::unordered_map<uint64_t, std::shared_ptr<Framebuffer>> framebuffers_in_use;

    std::vector<FreeObject> free_objects;

    uint64_t next_general_buffer_id;
    uint64_t next_index_buffer_id;
    uint64_t next_texture_id;
    uint64_t next_framebuffer_id;

    size_t bytes_committed = 0;
    size_t bytes_allocated = 0;

    //    template <typename T>
    //    uint64_t allocate_general_buffer(size_t size, std::string tag) {
    //        size_t byte_size = size * sizeof(T);
    //
    //        if (byte_size < MAX_BUFFER_SIZE_CLASS) {
    //            byte_size = upper_power_of_two(byte_size);
    //        }
    //
    //        let now = Instant::now();
    //
    //        for (int free_object_index = 0; free_object_index < free_objects.size(); free_object_index++) {
    //            if (free_objects[free_object_index].kind == FreeObjectKind::GeneralBuffer) {
    //                if (allocation.size == byte_size && (now - *timestamp).as_secs_f32() >= REUSE_TIME) {
    //                } else {
    //                    continue;
    //                }
    //            }
    //
    //            uint64_t id = free_objects[free_object_index].id;
    //            BufferAllocation allocation;
    //
    //            allocation.tag = tag;
    //            bytes_committed += allocation.size;
    //            general_buffers_in_use.insert(id, allocation);
    //            return id;
    //        }
    //
    //        // Create a new buffer.
    //
    //        auto buffer =
    //            driver->create_buffer(BufferType::Storage, byte_size, MemoryProperty::HostVisibleAndCoherent, tag);
    //
    //        auto id = next_general_buffer_id;
    //        next_general_buffer_id += 1;
    //
    //        general_buffers_in_use.insert(id, BufferAllocation{buffer, size : byte_size, tag});
    //        bytes_allocated += byte_size;
    //        bytes_committed += byte_size;
    //
    //        return id;
    //    }
};

} // namespace Pathfinder

#endif // PATHFINDER_ALLOCATOR_H
