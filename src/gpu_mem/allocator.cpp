#include "allocator.h"

namespace Pathfinder {

uint64_t GpuMemoryAllocator::allocate_general_buffer(size_t byte_size, const std::string& tag) {
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

std::shared_ptr<Buffer> GpuMemoryAllocator::get_general_buffer(uint64_t id) {
    if (general_buffers_in_use.find(id) == general_buffers_in_use.end()) {
        Logger::error("Tried to get nonexistent general buffer!", "GpuMemoryAllocator");
        return nullptr;
    }

    return general_buffers_in_use[id].buffer;
}

} // namespace Pathfinder
