#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "base.h"
#include "buffer.h"
#include "texture.h"

namespace Pathfinder {

struct DescriptorLayout {
    uint32_t binding{};
    ShaderStage stage{};
    DescriptorType type{};
};

class DescriptorSetLayout {
    friend class DeviceGl;
    friend class DeviceMtl;

public:
    virtual ~DescriptorSetLayout() = default;

    const std::map<uint32_t, DescriptorLayout>& get_descriptor_layouts() const {
        return layouts;
    }

    const DescriptorLayout& get_descriptor_layout(uint32_t binding) const {
        return layouts.at(binding);
    }

protected:
    explicit DescriptorSetLayout(const std::vector<DescriptorLayout>& _layouts) {
        for (const auto& l : _layouts) {
            layouts[l.binding] = l;
        }
    }

    std::map<uint32_t, DescriptorLayout> layouts;
};

struct Descriptor {
    /// Binding point.
    uint32_t binding{};

    DescriptorType type{};

    /// 1. For buffer
    std::shared_ptr<Buffer> buffer;
    uint64_t buffer_offset{}; // Must be a multiple of minUniformBufferOffsetAlignment.
    uint64_t buffer_range{};  // Zero means whole size.

    /// 2. For compute image & sampling
    std::shared_ptr<Texture> texture;

    /// 3. For sampling
    std::shared_ptr<Sampler> sampler;

    static Descriptor uniform(uint32_t binding,
                              const std::shared_ptr<Buffer>& buffer = nullptr,
                              uint64_t buffer_offset = 0,
                              uint64_t buffer_range = 0) {
        if (buffer && buffer->get_type() != BufferType::Uniform) {
            throw std::runtime_error(std::string("Mismatched buffer type when creating a descriptor!"));
        }

        Descriptor desc{};
        desc.type = DescriptorType::UniformBuffer;
        desc.binding = binding;
        desc.buffer = buffer;
        desc.buffer_offset = buffer_offset;
        desc.buffer_range = buffer_range;

        if (buffer_range == 0 && buffer) {
            desc.buffer_range = buffer->get_size();
        }

        return desc;
    }

    static Descriptor sampled(uint32_t binding,
                              const std::shared_ptr<Texture>& texture = nullptr,
                              const std::shared_ptr<Sampler>& sampler = nullptr) {
        Descriptor desc{};
        desc.type = DescriptorType::Sampler;
        desc.binding = binding;
        desc.texture = texture;
        desc.sampler = sampler;

        return desc;
    }

    static Descriptor storage(uint32_t binding,
                              const std::shared_ptr<Buffer>& buffer = nullptr,
                              uint64_t buffer_offset = 0,
                              uint64_t buffer_range = 0) {
        if (buffer && buffer->get_type() != BufferType::Storage) {
            throw std::runtime_error(std::string("Mismatched buffer type when creating a descriptor!"));
        }

        Descriptor desc{};
        desc.type = DescriptorType::StorageBuffer;
        desc.binding = binding;
        desc.buffer = buffer;
        desc.buffer_offset = buffer_offset;
        desc.buffer_range = buffer_range;

        return desc;
    }

    static Descriptor image(uint32_t binding, const std::shared_ptr<Texture>& texture = nullptr) {
        Descriptor desc{};
        desc.type = DescriptorType::Image;
        desc.binding = binding;
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
    friend class DeviceMtl;

public:
    virtual ~DescriptorSet() = default;

    void add_or_update(const std::vector<Descriptor>& _descriptors) {
        for (auto& d : _descriptors) {
            descriptors[d.binding] = d;
        }

        dirty = true;
    }

    const std::map<uint32_t, Descriptor>& get_descriptors() const {
        return descriptors;
    }

    std::shared_ptr<DescriptorSetLayout> get_layout() const {
        return layout_;
    }

protected:
    explicit DescriptorSet(const std::shared_ptr<DescriptorSetLayout>& layout) : layout_(layout) {}

protected:
    std::shared_ptr<DescriptorSetLayout> layout_;

    /// Binding point is used as the hashing key.
    std::map<uint32_t, Descriptor> descriptors;

    bool dirty = false;
};

} // namespace Pathfinder
