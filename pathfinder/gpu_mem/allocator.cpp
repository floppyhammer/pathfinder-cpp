#include "allocator.h"

namespace Pathfinder {

uint64_t GpuMemoryAllocator::allocate_buffer(size_t byte_size, BufferType type, const std::string& tag) {
    if (byte_size < MAX_BUFFER_SIZE_CLASS) {
        byte_size = upper_power_of_two(byte_size);
    }

    auto descriptor = BufferDescriptor{type, byte_size, MemoryProperty::HostVisibleAndCoherent};

    // Try to find a free object in the idle pool.
    // Anything in idle_pool is confirmed safe by frame-delay.
    for (int i = 0; i < (int)idle_pool.size(); i++) {
        auto& free_obj = idle_pool[i];

        if (free_obj.kind == FreeObjectKind::Buffer && free_obj.buffer_allocation.descriptor == descriptor) {
            uint64_t id = free_obj.id;
            BufferAllocation allocation = free_obj.buffer_allocation;

            idle_pool.erase(idle_pool.begin() + i);

            allocation.tag = tag;
            allocation.buffer->set_label(tag);

            bytes_committed += byte_size;
            active_buffers[id] = allocation;

            return id;
        }
    }

    auto buffer = device->create_buffer(descriptor, tag);
    auto id = next_buffer_id++;
    active_buffers[id] = BufferAllocation{buffer, descriptor, tag};

    bytes_allocated += byte_size;
    bytes_committed += byte_size;

    return id;
}

uint64_t GpuMemoryAllocator::allocate_texture(Vec2I size, TextureFormat format, const std::string& tag) {
    assert(!size.is_any_zero());

    auto descriptor = TextureDescriptor{size, format};
    auto byte_size = descriptor.byte_size();

    // Try to find a free object in the idle pool.
    for (int i = 0; i < (int)idle_pool.size(); i++) {
        auto& free_obj = idle_pool[i];

        if (free_obj.kind == FreeObjectKind::Texture && free_obj.texture_allocation.descriptor == descriptor) {
            uint64_t id = free_obj.id;
            auto allocation = free_obj.texture_allocation;

            idle_pool.erase(idle_pool.begin() + i);

            allocation.tag = tag;
            allocation.texture->set_label(tag);

            bytes_committed += byte_size;
            active_textures[id] = allocation;

            return id;
        }
    }

    auto texture = device->create_texture(descriptor, tag);
    auto id = next_texture_id++;
    active_textures[id] = TextureAllocation{texture, descriptor, tag};

    bytes_allocated += byte_size;
    bytes_committed += byte_size;

    return id;
}

void GpuMemoryAllocator::free_buffer(uint64_t id) {
    auto it = active_buffers.find(id);
    if (it == active_buffers.end()) {
        Logger::error("Attempted to free unallocated buffer!");
        return;
    }

    auto allocation = it->second;
    active_buffers.erase(it);

    bytes_committed -= allocation.buffer->get_size();

    FreeObject free_obj;
    free_obj.timestamp = std::chrono::steady_clock::now();
    free_obj.kind = FreeObjectKind::Buffer;
    free_obj.id = id;
    free_obj.buffer_allocation = allocation;

    // Put into current frame bucket.
    pending_buckets[current_frame_index % MAX_FRAMES_IN_FLIGHT].objects.push_back(free_obj);
}

void GpuMemoryAllocator::free_texture(uint64_t id) {
    auto it = active_textures.find(id);
    if (it == active_textures.end()) {
        Logger::error("Attempted to free unallocated texture!");
        return;
    }

    auto allocation = it->second;
    active_textures.erase(it);

    bytes_committed -= allocation.descriptor.byte_size();

    FreeObject free_obj;
    free_obj.timestamp = std::chrono::steady_clock::now();
    free_obj.kind = FreeObjectKind::Texture;
    free_obj.id = id;
    free_obj.texture_allocation = allocation;

    // Put into current frame bucket.
    pending_buckets[current_frame_index % MAX_FRAMES_IN_FLIGHT].objects.push_back(free_obj);
}

std::shared_ptr<Buffer> GpuMemoryAllocator::get_buffer(uint64_t id) {
    auto it = active_buffers.find(id);
    if (it == active_buffers.end()) {
        Logger::error("Attempted to get unallocated buffer!");
        return nullptr;
    }
    return it->second.buffer;
}

std::shared_ptr<Texture> GpuMemoryAllocator::get_texture(uint64_t id) {
    auto it = active_textures.find(id);
    if (it == active_textures.end()) {
        Logger::error("Attempted to get unallocated texture!");
        return nullptr;
    }
    return it->second.texture;
}

void GpuMemoryAllocator::begin_frame() {
    // Move to next frame.
    current_frame_index++;

    // Reclaim the bucket we are about to overwrite.
    // This bucket contains objects that have been "cooling down" for MAX_FRAMES_IN_FLIGHT frames.
    auto& bucket = pending_buckets[current_frame_index % MAX_FRAMES_IN_FLIGHT];
    if (!bucket.objects.empty()) {
        idle_pool.insert(idle_pool.end(), bucket.objects.begin(), bucket.objects.end());
        bucket.objects.clear();
    }
}

void GpuMemoryAllocator::purge_if_needed() {
    auto now = std::chrono::steady_clock::now();
    bool purge_happened = false;

    for (auto it = idle_pool.begin(); it != idle_pool.end();) {
        std::chrono::duration<double> duration = now - it->timestamp;
        if (duration.count() >= DECAY_TIME) {
            if (it->kind == FreeObjectKind::Buffer) {
                bytes_allocated -= it->buffer_allocation.buffer->get_size();
            } else {
                bytes_allocated -= it->texture_allocation.descriptor.byte_size();
            }
            it = idle_pool.erase(it);
            purge_happened = true;
        } else {
            ++it;
        }
    }

    if (purge_happened) print_info();
}

void GpuMemoryAllocator::print_info() {
    size_t texture_count = active_textures.size();
    size_t buffer_count = active_buffers.size();

    size_t idle_count = idle_pool.size();
    size_t pending_count = 0;
    for (const auto& bucket : pending_buckets) {
        pending_count += bucket.objects.size();
    }

    Logger::debug("Current status: ALLOCATED " + std::to_string(int(bytes_allocated / 1024.f)) + " KB | COMMITTED " +
                  std::to_string(int(bytes_committed / 1024.f)) + " KB | Active textures " +
                  std::to_string(texture_count) + " | Active buffers " + std::to_string(buffer_count) +
                  " | Free (Idle/Pending) " + std::to_string(idle_count) + "/" + std::to_string(pending_count));

    for (auto& allocation : active_textures) {
        Logger::debug("Active texture " + std::to_string(allocation.first) + ": " + allocation.second.tag + " - " +
                      std::to_string(int(allocation.second.descriptor.byte_size() / 1024.f)) + " KB");
    }

    for (auto& allocation : active_buffers) {
        Logger::debug("Active Buffer " + std::to_string(allocation.first) + ": " + allocation.second.tag + " - " +
                      std::to_string(int(allocation.second.descriptor.size / 1024.f)) + " KB");
    }
}

} // namespace Pathfinder
