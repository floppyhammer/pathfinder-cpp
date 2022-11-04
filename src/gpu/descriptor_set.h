#ifndef PATHFINDER_GPU_DESCRIPTOR_SET_H
#define PATHFINDER_GPU_DESCRIPTOR_SET_H

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "buffer.h"
#include "data.h"
#include "texture.h"

namespace Pathfinder {

struct Descriptor {
    DescriptorType type = DescriptorType::Max;

    ShaderStage stage = ShaderStage::Max;

    /// Binding point.
    uint32_t binding{};

    /// For compatibility with lower versions of OpenGL.
    std::string binding_name;

    /// Only one of these is used.
    std::shared_ptr<Buffer> buffer;
    std::shared_ptr<Texture> texture;

    static Descriptor uniform(uint32_t binding,
                              ShaderStage stage,
                              const std::string& binding_name,
                              const std::shared_ptr<Buffer>& buffer = nullptr) {
        if (buffer && buffer->type != BufferType::Uniform) {
            throw std::runtime_error(std::string("Mismatched buffer type when creating a descriptor!"));
        }

        return {DescriptorType::UniformBuffer, stage, binding, binding_name, buffer, nullptr};
    }

    static Descriptor sampler(uint32_t binding,
                              ShaderStage stage,
                              const std::string& binding_name,
                              const std::shared_ptr<Texture>& texture = nullptr) {
        return {DescriptorType::Sampler, stage, binding, binding_name, nullptr, texture};
    }

    static Descriptor storage(uint32_t binding, ShaderStage stage, const std::shared_ptr<Buffer>& buffer = nullptr) {
        if (buffer && buffer->type != BufferType::Storage) {
            throw std::runtime_error(std::string("Mismatched buffer type when creating a descriptor!"));
        }

        return {DescriptorType::StorageBuffer, stage, binding, "", buffer, nullptr};
    }

    static Descriptor image(uint32_t binding,
                            ShaderStage stage,
                            const std::string& binding_name,
                            const std::shared_ptr<Texture>& texture = nullptr) {
        return {DescriptorType::Image, stage, binding, binding_name, nullptr, texture};
    }
};

/**
 * This acts as both `set layout` and `set`.
 * If any of the contained descriptors has null `buffer`s/`texture`s, it is a `layout`.
 * Otherwise, it's a `set`.
 */
class DescriptorSet {
public:
    inline void add_or_update(const std::vector<Descriptor>& _descriptors) {
        for (auto& d : _descriptors) {
            descriptors[d.binding] = d;
        }
    }

    inline const std::unordered_map<uint32_t, Descriptor>& get_descriptors() const {
        return descriptors;
    }

protected:
    /// Binding point is used as the hashing key.
    std::unordered_map<uint32_t, Descriptor> descriptors;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_DESCRIPTOR_SET_H
