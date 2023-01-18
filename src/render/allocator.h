#ifndef PATHFINDER_ALLOCATOR_H
#define PATHFINDER_ALLOCATOR_H

#include <chrono>
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
    GpuMemoryAllocator(std::shared_ptr<Driver> _driver) : driver(_driver) {}

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

    template <typename T>
    uint64_t allocate_general_buffer(size_t size, const std::string& tag) {
        size_t byte_size = size * sizeof(T);

        if (byte_size < MAX_BUFFER_SIZE_CLASS) {
            byte_size = upper_power_of_two(byte_size);
        }

        auto now = std::chrono::steady_clock::now();

        // Try to find a free object.
        for (int free_object_index = 0; free_object_index < free_objects.size(); free_object_index++) {
            auto& free_obj = free_objects[free_object_index];

            if (free_obj.kind == FreeObjectKind::GeneralBuffer) {
                std::chrono::duration<double> duration = now - free_obj.timestamp;
                auto elapsed_time_in_s = duration.count() * 1.0e6;

                if (free_obj.general_allocation.buffer->get_size() == byte_size && elapsed_time_in_s >= REUSE_TIME) {
                } else {
                    continue;
                }
            }

            // Reuse this free object.
            free_objects.erase(free_objects.begin() + free_object_index);
            uint64_t id = free_obj.id;
            BufferAllocation allocation = free_obj.general_allocation;

            // Update tag.
            allocation.tag = tag;
            bytes_committed += allocation.buffer->get_size();

            general_buffers_in_use[id] = allocation;

            return id;
        }

        // Create a new buffer.

        auto buffer =
            driver->create_buffer(BufferType::Storage, byte_size, MemoryProperty::HostVisibleAndCoherent, tag);

        auto id = next_general_buffer_id;
        next_general_buffer_id += 1;

        general_buffers_in_use[id] = BufferAllocation{buffer, tag};

        bytes_allocated += byte_size;
        bytes_committed += byte_size;

        return id;
    }

    void free_general_buffer(uint64_t id) {
        if (general_buffers_in_use.find(id) == general_buffers_in_use.end()) {
            Logger::error("Tried to free nonexistent general buffer!", "GpuMemoryAllocator");
            return;
        }

        auto allocation = general_buffers_in_use[id];

        general_buffers_in_use.erase(id);

        bytes_committed -= allocation.buffer->get_size();

        // Put the buffer back to the free objects.

        FreeObject free_obj;
        free_obj.timestamp = std::chrono::steady_clock::now();
        free_obj.kind = FreeObjectKind::GeneralBuffer;
        free_obj.id = id;
        free_obj.general_allocation = allocation;

        free_objects.push_back(free_obj);
    }

    std::shared_ptr<Buffer> get_general_buffer(uint64_t id) {
        if (general_buffers_in_use.find(id) == general_buffers_in_use.end()) {
            Logger::error("Tried to get nonexistent general buffer!", "GpuMemoryAllocator");
            return nullptr;
        }

        return general_buffers_in_use[id].buffer;
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_ALLOCATOR_H
