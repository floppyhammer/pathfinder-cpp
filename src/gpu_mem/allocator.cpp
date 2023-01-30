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

    auto buffer = driver->create_buffer({BufferType::Storage, byte_size, MemoryProperty::HostVisibleAndCoherent, tag});

    auto id = next_general_buffer_id;
    next_general_buffer_id += 1;

    general_buffers_in_use[id] = BufferAllocation{buffer, tag};

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

        if (free_obj.kind == FreeObjectKind::Texture) {
            if (free_obj.texture_allocation.descriptor == descriptor) {
            } else {
                // Check next one.
                continue;
            }
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

    auto texture = driver->create_texture(descriptor);
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

        if (free_obj.kind == FreeObjectKind::Framebuffer) {
            if (free_obj.framebuffer_allocation.descriptor == descriptor) {
            } else {
                // Check next one.
                continue;
            }
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
            driver->create_render_pass(format, AttachmentLoadOp::Clear, "GpuMemoryAllocator render pass");
        render_pass_cache[format] = render_pass;
    }

    auto texture = driver->create_texture(descriptor);
    auto framebuffer = driver->create_framebuffer(render_pass_cache[format], texture, tag);

    auto id = next_framebuffer_id;
    next_framebuffer_id += 1;

    framebuffers_in_use[id] = FramebufferAllocation{framebuffer, descriptor, tag};

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
            case FreeObjectKind::GeneralBuffer: {
                Logger::debug("Purging general buffer...", "GpuMemoryAllocator");
                free_objects.erase(free_objects.begin());
                bytes_allocated -= oldest_free_obj.general_allocation.buffer->get_size();

                purge_happened = true;
            } break;
            case FreeObjectKind::Texture: {
                Logger::debug("Purging texture...", "GpuMemoryAllocator");
                free_objects.erase(free_objects.begin());
                bytes_allocated -= oldest_free_obj.texture_allocation.descriptor.byte_size();

                purge_happened = true;
            } break;
            case FreeObjectKind::Framebuffer: {
                Logger::debug("Purging framebuffer...", "GpuMemoryAllocator");
                free_objects.erase(free_objects.begin());
                bytes_allocated -= oldest_free_obj.framebuffer_allocation.descriptor.byte_size();

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

std::shared_ptr<Buffer> GpuMemoryAllocator::get_general_buffer(uint64_t id) {
    if (general_buffers_in_use.find(id) == general_buffers_in_use.end()) {
        Logger::error("Attempted to get nonexistent general buffer!", "GpuMemoryAllocator");
        return nullptr;
    }

    return general_buffers_in_use[id].buffer;
}

std::shared_ptr<Texture> GpuMemoryAllocator::get_texture(uint64_t id) {
    if (textures_in_use.find(id) == textures_in_use.end()) {
        Logger::error("Attempted to get nonexistent texture!", "GpuMemoryAllocator");
        return nullptr;
    }

    return textures_in_use[id].texture;
}

std::shared_ptr<Framebuffer> GpuMemoryAllocator::get_framebuffer(uint64_t id) {
    if (framebuffers_in_use.find(id) == framebuffers_in_use.end()) {
        Logger::error("Attempted to get nonexistent framebuffer!", "GpuMemoryAllocator");
        return nullptr;
    }

    return framebuffers_in_use[id].framebuffer;
}

} // namespace Pathfinder
