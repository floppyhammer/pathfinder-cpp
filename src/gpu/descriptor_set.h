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

    /// Only one is used.
    std::shared_ptr<Buffer> buffer;
    std::shared_ptr<Texture> texture;
};

/**
 * This acts as both `set layout` and `set`.
 * If any of the contained descriptors has null [buffer]s/[texture]s, we can only use it as a layout.
 * Otherwise, it's a `set`.
 */
class DescriptorSet {
public:
    inline void add_or_update_uniform(ShaderStage stage,
                                      uint32_t binding,
                                      std::string binding_name,
                                      std::shared_ptr<Buffer> buffer = nullptr) {
        if (buffer) {
            if (buffer->type != BufferType::Uniform) {
                throw std::runtime_error(std::string("Mismatched buffer type when adding a descriptor!"));
            }
        }

        Descriptor descriptor{DescriptorType::UniformBuffer, stage, binding, binding_name, buffer, nullptr};
        descriptors[descriptor.binding] = descriptor;
    }

    inline void add_or_update_sampler(ShaderStage stage,
                                      uint32_t binding,
                                      std::string binding_name,
                                      std::shared_ptr<Texture> texture = nullptr) {
        Descriptor descriptor{DescriptorType::Sampler, stage, binding, binding_name, nullptr, texture};
        descriptors[descriptor.binding] = descriptor;
    }

    inline void add_or_update_storage(ShaderStage stage, uint32_t binding, std::shared_ptr<Buffer> buffer = nullptr) {
        if (buffer) {
            if (buffer->type != BufferType::Storage) {
                throw std::runtime_error(std::string("Mismatched buffer type when adding a descriptor!"));
            }
        }

        Descriptor descriptor{DescriptorType::StorageBuffer, stage, binding, "", buffer, nullptr};
        descriptors[descriptor.binding] = descriptor;
    }

    inline void add_or_update_image(ShaderStage stage,
                                    uint32_t binding,
                                    std::string binding_name,
                                    std::shared_ptr<Texture> texture = nullptr) {
        Descriptor descriptor{DescriptorType::Image, stage, binding, binding_name, nullptr, texture};
        descriptors[descriptor.binding] = descriptor;
    }

    inline const std::unordered_map<uint32_t, Descriptor> &get_descriptors() const {
        return descriptors;
    }

protected:
    std::unordered_map<uint32_t, Descriptor> descriptors;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_DESCRIPTOR_SET_H
