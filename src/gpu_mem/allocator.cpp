#include "allocator.h"

namespace Pathfinder {

uint64_t GpuMemoryAllocator::allocate_buffer(size_t byte_size, BufferType type, const std::string& tag) {
    if (byte_size < MAX_BUFFER_SIZE_CLASS) {
        byte_size = upper_power_of_two(byte_size);
    }

    auto descriptor = BufferDescriptor{type, byte_size, MemoryProperty::HostVisibleAndCoherent, tag};

    auto now = std::chrono::steady_clock::now();

    // Try to find a free object.
    for (int free_object_index = 0; free_object_index < free_objects.size(); free_object_index++) {
        auto free_obj = free_objects[free_object_index];

        bool found_free_obj = false;

        if (free_obj.kind == FreeObjectKind::Buffer && free_obj.buffer_allocation.descriptor == descriptor) {
            std::chrono::duration<double> duration = now - free_obj.timestamp;

            // Check if this free buffer can be reused.
            if (duration.count() >= REUSE_TIME) {
                found_free_obj = true;
            }
        }

        if (!found_free_obj) {
            continue;
        }

        // Reuse this free object.
        free_objects.erase(free_objects.begin() + free_object_index);

        uint64_t id = free_obj.id;
        BufferAllocation allocation = free_obj.buffer_allocation;

        // Update tag.
        // TODO: also update GPU debug marker.
        allocation.tag = tag;

        bytes_committed += byte_size;
        buffers_in_use[id] = allocation;

        return id;
    }

    // Create a new buffer.

    auto buffer = device->create_buffer(descriptor);

    auto id = next_buffer_id;
    next_buffer_id += 1;

    buffers_in_use[id] = BufferAllocation{buffer, descriptor, tag};

    bytes_allocated += byte_size;
    bytes_committed += byte_size;

    return id;
}

uint64_t GpuMemoryAllocator::allocate_texture(Vec2I size, TextureFormat format, const std::string& tag) {
    auto descriptor = TextureDescriptor{size, format, tag};

    auto byte_size = descriptor.byte_size();

    // Try to find a free object.
    for (int free_object_index = 0; free_object_index < free_objects.size(); free_object_index++) {
        auto free_obj = free_objects[free_object_index];

        if (!(free_obj.kind == FreeObjectKind::Texture && free_obj.texture_allocation.descriptor == descriptor)) {
            continue;
        }

        // Reuse this free object.
        free_objects.erase(free_objects.begin() + free_object_index);

        uint64_t id = free_obj.id;
        auto allocation = free_obj.texture_allocation;

        allocation.tag = tag;

        bytes_committed += byte_size;
        textures_in_use[id] = allocation;

        return id;
    }

    // Create a new texture.

    auto texture = device->create_texture(descriptor);
    auto id = next_texture_id;
    next_texture_id += 1;

    textures_in_use[id] = TextureAllocation{texture, descriptor, tag};

    bytes_allocated += byte_size;
    bytes_committed += byte_size;

    return id;
}

uint64_t GpuMemoryAllocator::allocate_framebuffer(Vec2I size, TextureFormat format, const std::string& tag) {
    auto descriptor = TextureDescriptor{size, format, tag};

    auto byte_size = descriptor.byte_size();

    // Try to find a free object.
    for (int free_object_index = 0; free_object_index < free_objects.size(); free_object_index++) {
        auto free_obj = free_objects[free_object_index];

        if (!(free_obj.kind == FreeObjectKind::Framebuffer &&
              free_obj.framebuffer_allocation.descriptor == descriptor)) {
            continue;
        }

        // Reuse this free object.
        free_objects.erase(free_objects.begin() + free_object_index);

        uint64_t id = free_obj.id;
        auto allocation = free_obj.framebuffer_allocation;

        allocation.tag = tag;

        bytes_committed += byte_size;
        framebuffers_in_use[id] = allocation;

        return id;
    }

    // Create a new framebuffer.

    if (render_pass_cache.find(format) == render_pass_cache.end()) {
        auto render_pass =
            device->create_render_pass(format, AttachmentLoadOp::Clear, "GpuMemoryAllocator dummy render pass");
        render_pass_cache[format] = render_pass;
    }

    auto texture = device->create_texture(descriptor);
    auto framebuffer = device->create_framebuffer(render_pass_cache[format], texture, tag);

    auto id = next_framebuffer_id;
    next_framebuffer_id += 1;

    framebuffers_in_use[id] = FramebufferAllocation{framebuffer, descriptor, tag};

    bytes_allocated += byte_size;
    bytes_committed += byte_size;

    return id;
}

void GpuMemoryAllocator::free_buffer(uint64_t id) {
    if (buffers_in_use.find(id) == buffers_in_use.end()) {
        Logger::error("Attempted to free unallocated buffer!", "GpuMemoryAllocator");
        return;
    }

    auto allocation = buffers_in_use[id];

    buffers_in_use.erase(id);

    bytes_committed -= allocation.buffer->get_size();

    // Put the buffer back to the free objects.

    FreeObject free_obj;
    free_obj.timestamp = std::chrono::steady_clock::now();
    free_obj.kind = FreeObjectKind::Buffer;
    free_obj.id = id;
    free_obj.buffer_allocation = allocation;

    free_objects.push_back(free_obj);
}

void GpuMemoryAllocator::free_texture(uint64_t id) {
    if (textures_in_use.find(id) == textures_in_use.end()) {
        Logger::error("Attempted to free unallocated texture!", "GpuMemoryAllocator");
        return;
    }

    auto allocation = textures_in_use[id];

    textures_in_use.erase(id);

    auto byte_size = allocation.descriptor.byte_size();
    bytes_committed -= byte_size;

    FreeObject free_obj;
    free_obj.timestamp = std::chrono::steady_clock::now();
    free_obj.kind = FreeObjectKind::Texture;
    free_obj.id = id;
    free_obj.texture_allocation = allocation;

    free_objects.push_back(free_obj);
}

void GpuMemoryAllocator::free_framebuffer(uint64_t id) {
    if (framebuffers_in_use.find(id) == framebuffers_in_use.end()) {
        Logger::error("Attempted to free unallocated framebuffer!", "GpuMemoryAllocator");
        return;
    }

    auto allocation = framebuffers_in_use[id];

    framebuffers_in_use.erase(id);

    auto byte_size = allocation.descriptor.byte_size();
    bytes_committed -= byte_size;

    FreeObject free_obj;
    free_obj.timestamp = std::chrono::steady_clock::now();
    free_obj.kind = FreeObjectKind::Framebuffer;
    free_obj.id = id;
    free_obj.framebuffer_allocation = allocation;

    free_objects.push_back(free_obj);
}

std::shared_ptr<Buffer> GpuMemoryAllocator::get_buffer(uint64_t id) {
    if (buffers_in_use.find(id) == buffers_in_use.end()) {
        Logger::error("Attempted to get unallocated buffer!", "GpuMemoryAllocator");
        return nullptr;
    }

    return buffers_in_use[id].buffer;
}

std::shared_ptr<Texture> GpuMemoryAllocator::get_texture(uint64_t id) {
    if (textures_in_use.find(id) == textures_in_use.end()) {
        Logger::error("Attempted to get unallocated texture!", "GpuMemoryAllocator");
        return nullptr;
    }

    return textures_in_use[id].texture;
}

std::shared_ptr<Framebuffer> GpuMemoryAllocator::get_framebuffer(uint64_t id) {
    if (framebuffers_in_use.find(id) == framebuffers_in_use.end()) {
        Logger::error("Attempted to get unallocated framebuffer!", "GpuMemoryAllocator");
        return nullptr;
    }

    return framebuffers_in_use[id].framebuffer;
}

void GpuMemoryAllocator::purge_if_needed() {
    auto now = std::chrono::steady_clock::now();

    bool purge_happened = false;

    while (true) {
        if (free_objects.empty()) {
            break;
        }

        // Has to be copied by value.
        auto oldest_free_obj = free_objects.front();

        std::chrono::duration<double> duration = now - oldest_free_obj.timestamp;
        // If the first free object has not decayed, so do the rest free objects.
        if (duration.count() < DECAY_TIME) {
            break;
        }

        switch (free_objects.front().kind) {
            case FreeObjectKind::Buffer: {
                Logger::info("Purging buffer: " + oldest_free_obj.buffer_allocation.tag, "GpuMemoryAllocator");

                free_objects.erase(free_objects.begin());
                bytes_allocated -= oldest_free_obj.buffer_allocation.buffer->get_size();

                purge_happened = true;
            } break;
            case FreeObjectKind::Texture: {
                Logger::info("Purging texture: " + oldest_free_obj.texture_allocation.tag, "GpuMemoryAllocator");

                free_objects.erase(free_objects.begin());
                bytes_allocated -= oldest_free_obj.texture_allocation.descriptor.byte_size();

                purge_happened = true;
            } break;
            case FreeObjectKind::Framebuffer: {
                Logger::info("Purging framebuffer: " + oldest_free_obj.framebuffer_allocation.tag,
                             "GpuMemoryAllocator");

                free_objects.erase(free_objects.begin());
                bytes_allocated -= oldest_free_obj.framebuffer_allocation.descriptor.byte_size();

                purge_happened = true;
            } break;
            default:
                break;
        }
    }

    if (purge_happened) {
        print_info();
    }
}

void GpuMemoryAllocator::print_info() {
    size_t texture_count = textures_in_use.size();
    size_t framebuffer_count = framebuffers_in_use.size();
    size_t buffer_count = buffers_in_use.size();
    size_t free_object_count = free_objects.size();

    Logger::info("Current status: ALLOCATED " + std::to_string(int(bytes_allocated / 1024.f)) + " KB | COMMITTED " +
                     std::to_string(int(bytes_committed / 1024.f)) + " KB | Textures " + std::to_string(texture_count) +
                     " | Framebuffers " + std::to_string(framebuffer_count) + " | Buffers " +
                     std::to_string(buffer_count) + " | Free objects " + std::to_string(free_object_count),
                 "GpuMemoryAllocator");

    for (auto& allocation : textures_in_use) {
        Logger::info("Texture " + std::to_string(allocation.first) + ": " + allocation.second.tag + " - " +
                         std::to_string(int(allocation.second.descriptor.byte_size() / 1024.f)) + " KB",
                     "GpuMemoryAllocator");
    }

    for (auto& allocation : framebuffers_in_use) {
        Logger::info("Framebuffer " + std::to_string(allocation.first) + ": " + allocation.second.tag + " - " +
                         std::to_string(int(allocation.second.descriptor.byte_size() / 1024.f)) + " KB",
                     "GpuMemoryAllocator");
    }

    for (auto& allocation : buffers_in_use) {
        Logger::info("Buffer " + std::to_string(allocation.first) + ": " + allocation.second.tag + " - " +
                         std::to_string(int(allocation.second.descriptor.size / 1024.f)) + " KB",
                     "GpuMemoryAllocator");
    }
}

} // namespace Pathfinder
