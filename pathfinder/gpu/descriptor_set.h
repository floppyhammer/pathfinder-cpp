#ifndef PATHFINDER_GPU_DESCRIPTOR_SET_H
#define PATHFINDER_GPU_DESCRIPTOR_SET_H

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "base.h"
#include "buffer.h"
#include "texture.h"

namespace Pathfinder {

struct Descriptor {
    DescriptorType type{};

    ShaderStage stage{};

    /// Binding point.
    uint32_t binding{};

    /// For compatibility with lower versions of OpenGL.
    std::string binding_name;

    /// 1. For buffer
    std::shared_ptr<Buffer> buffer;
    uint64_t buffer_offset{}; // Must be a multiple of minUniformBufferOffsetAlignment.
    uint64_t buffer_range{};

    /// 2. For compute image & sampling
    std::shared_ptr<Texture> texture;

    /// 3. For sampling
    std::shared_ptr<Sampler> sampler;

    static Descriptor uniform(uint32_t binding,
                              ShaderStage stage,
                              const std::string& binding_name,
                              const std::shared_ptr<Buffer>& buffer = nullptr,
                              uint64_t buffer_offset = 0,
                              uint64_t buffer_range = 0) {
        if (buffer && buffer->get_type() != BufferType::Uniform) {
            throw std::runtime_error(std::string("Mismatched buffer type when creating a descriptor!"));
        }

        Descriptor desc{};
        desc.type = DescriptorType::UniformBuffer;
        desc.binding = binding;
        desc.binding_name = binding_name;
        desc.stage = stage;
        desc.buffer = buffer;
        desc.buffer_offset = buffer_offset;
        desc.buffer_range = buffer_range;

        if (buffer_range == 0 && buffer) {
            desc.buffer_range = buffer->get_size();
        }

        return desc;
    }

    static Descriptor sampled(uint32_t binding,
                              ShaderStage stage,
                              const std::string& binding_name,
                              const std::shared_ptr<Texture>& texture = nullptr,
                              const std::shared_ptr<Sampler>& sampler = nullptr) {
        Descriptor desc{};
        desc.type = DescriptorType::Sampler;
        desc.stage = stage;
        desc.binding = binding;
        desc.binding_name = binding_name;
        desc.texture = texture;
        desc.sampler = sampler;

        return desc;
    }

    static Descriptor storage(uint32_t binding,
                              ShaderStage stage,
                              const std::shared_ptr<Buffer>& buffer = nullptr,
                              uint64_t buffer_offset = 0,
                              uint64_t buffer_range = 0) {
        if (buffer && buffer->get_type() != BufferType::Storage) {
            throw std::runtime_error(std::string("Mismatched buffer type when creating a descriptor!"));
        }

        Descriptor desc{};
        desc.type = DescriptorType::StorageBuffer;
        desc.binding = binding;
        desc.stage = stage;
        desc.buffer = buffer;
        desc.buffer_offset = buffer_offset;
        desc.buffer_range = buffer_range;

        return desc;
    }

    static Descriptor image(uint32_t binding,
                            ShaderStage stage,
                            const std::string& binding_name,
                            const std::shared_ptr<Texture>& texture = nullptr) {
        Descriptor desc{};
        desc.type = DescriptorType::Image;
        desc.stage = stage;
        desc.binding = binding;
        desc.binding_name = binding_name;
        desc.texture = texture;

        return desc;
    }
};

/**
 * This acts as both `set layout` and `set`.
 * If any of the contained descriptors has null `buffer`s/`texture`s, it is a `layout`.
 * Otherwise, it's a `set`.
 */
class DescriptorSet {
    friend class DeviceGl;

public:
    virtual ~DescriptorSet() = default;

    void add_or_update(const std::vector<Descriptor>& _descriptors) {
        for (auto& d : _descriptors) {
            descriptors[d.binding] = d;
        }

        dirty = true;
    }

    const std::unordered_map<uint32_t, Descriptor>& get_descriptors() const {
        return descriptors;
    }

protected:
    DescriptorSet() = default;

protected:
    /// Binding point is used as the hashing key.
    std::unordered_map<uint32_t, Descriptor> descriptors;

    bool dirty = false;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_DESCRIPTOR_SET_H
