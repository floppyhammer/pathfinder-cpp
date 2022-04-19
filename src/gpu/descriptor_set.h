#ifndef PATHFINDER_GPU_DESCRIPTOR_SET_H
#define PATHFINDER_GPU_DESCRIPTOR_SET_H

#include "buffer.h"
#include "texture.h"

#include <cstdint>
#include <unordered_map>

namespace Pathfinder {
    enum class DescriptorType {
        UniformBuffer = 0,
        Texture = 20,
        GeneralBuffer = 40,
        Image = 60,
        Max = 80,
    };

    struct Descriptor {
        DescriptorType type;

        ShaderType stage;

        uint32_t binding{};
        std::string binding_name; // For compatibility with lower versions of OpenGL.

        // Only one is valid.
        std::shared_ptr<Buffer> buffer;
        std::shared_ptr<Texture> texture;
    };

    class DescriptorSet {
    public:
        inline void add_or_update_descriptor(const Descriptor &descriptor) {
            if (descriptor.buffer == nullptr && descriptor.texture == nullptr) return;

            descriptors[(uint32_t) descriptor.type + descriptor.binding] = descriptor;
        }

        inline const std::unordered_map<uint32_t, Descriptor> &get_descriptors() const {
            return descriptors;
        }

    protected:
        std::unordered_map<uint32_t, Descriptor> descriptors;
    };
}

#endif //PATHFINDER_GPU_DESCRIPTOR_SET_H
