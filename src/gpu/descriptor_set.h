#ifndef PATHFINDER_GPU_DESCRIPTOR_SET_H
#define PATHFINDER_GPU_DESCRIPTOR_SET_H

#include "data.h"
#include "buffer.h"
#include "texture.h"

#include <cstdint>
#include <unordered_map>

namespace Pathfinder {
    struct Descriptor {
        DescriptorType type = DescriptorType::Max;

        ShaderType stage = ShaderType::Max;

        /// Binding point.
        uint32_t binding{};
        std::string binding_name; // For compatibility with lower versions of OpenGL.

        // Only one is valid.
        std::shared_ptr<Buffer> buffer;
        std::shared_ptr<Texture> texture;
    };

    class DescriptorSet {
    public:
        inline void add_or_update_descriptor(const Descriptor &descriptor) {
            // We might use `set` as `set layout`. So, null buffer/texture is valid here.
            //if (descriptor.buffer == nullptr && descriptor.texture == nullptr) return;

            descriptors[descriptor.binding] = descriptor;
        }

        inline const std::unordered_map<uint32_t, Descriptor> &get_descriptors() const {
            return descriptors;
        }

    protected:
        std::unordered_map<uint32_t, Descriptor> descriptors;
    };
}

#endif //PATHFINDER_GPU_DESCRIPTOR_SET_H
