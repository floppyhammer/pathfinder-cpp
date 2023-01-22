#include "allocator.h"

namespace Pathfinder {

uint64_t GpuMemoryAllocator::allocate_general_buffer(size_t byte_size, const std::string& tag) {
    if (byte_size < MAX_BUFFER_SIZE_CLASS) {
        byte_size = upper_power_of_two(byte_size);
    }

    auto now = std::chrono::steady_clock::now();

    // Try to find a free object.
    for (int free_object_index = 0; free_object_index < free_objects.size(); free_object_index++) {
        auto free_obj = free_objects[free_object_index];

        if (free_obj.kind == FreeObjectKind::GeneralBuffer) {
            std::chrono::duration<double> duration = now - free_obj.timestamp;

            if (free_obj.general_allocation.buffer->get_size() == byte_size && duration.count() >= REUSE_TIME) {
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

        bytes_committed += byte_size;

        general_buffers_in_use[id] = allocation;

        return id;
    }

    // Create a new buffer.

    auto buffer = driver->create_buffer({BufferType::Storage, byte_size, MemoryProperty::HostVisibleAndCoherent}, tag);

    auto id = next_general_buffer_id;
    next_general_buffer_id += 1;

    general_buffers_in_use[id] = BufferAllocation{buffer, tag};

    bytes_allocated += byte_size;
    bytes_committed += byte_size;

    return id;
}

void GpuMemoryAllocator::free_general_buffer(uint64_t id) {
    if (general_buffers_in_use.find(id) == general_buffers_in_use.end()) {
        Logger::error("Attempted to free unallocated general buffer!", "GpuMemoryAllocator");
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

std::shared_ptr<Buffer> GpuMemoryAllocator::get_general_buffer(uint64_t id) {
    if (general_buffers_in_use.find(id) == general_buffers_in_use.end()) {
        Logger::error("Attempted to get nonexistent general buffer!", "GpuMemoryAllocator");
        return nullptr;
    }

    return general_buffers_in_use[id].buffer;
}

void GpuMemoryAllocator::purge_if_needed() {
    auto now = std::chrono::steady_clock::now();

    bool purge_happened = false;

    while (true) {
        if (free_objects.empty()) {
            break;
        }

        std::chrono::duration<double> duration = now - free_objects.front().timestamp;
        // If the first free object has not decayed, so do the rest free objects.
        if (duration.count() < DECAY_TIME) {
            break;
        }

        switch (free_objects.front().kind) {
            case FreeObjectKind::GeneralBuffer: {
                Logger::debug("Purging general buffer...", "GpuMemoryAllocator");
                auto allocation = free_objects.front();
                free_objects.erase(free_objects.begin());
                bytes_allocated -= allocation.general_allocation.buffer->get_size();

                purge_happened = true;
            } break;
            default:
                break;
        }
    }

    if (purge_happened) {
        Logger::info("GPU memory purged, current status: ALLOCATED " + std::to_string(bytes_allocated) +
                         " bytes | COMMITTED " + std::to_string(bytes_committed) + " bytes",
                     "GpuMemoryAllocator");
    }
}

} // namespace Pathfinder
