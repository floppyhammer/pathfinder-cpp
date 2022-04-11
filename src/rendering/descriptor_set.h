//
// Created by floppyhammer on 4/9/2022.
//

#ifndef PATHFINDER_DESCRIPTOR_SET_H
#define PATHFINDER_DESCRIPTOR_SET_H

#include "buffer.h"
#include "texture.h"

#include <cstdint>
#include <unordered_map>
#include <optional>

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

        uint32_t binding;
        std::string binding_name; // For uniforms.

        std::optional<std::shared_ptr<Buffer>> buffer;
        std::optional<std::shared_ptr<Texture>> texture;
    };

    class DescriptorSet {
    public:
        inline void add_descriptor(const Descriptor &descriptor) {
            if (descriptor.buffer == nullptr && descriptor.texture == nullptr) return;

            auto it = descriptors.find((uint32_t) descriptor.type + descriptor.binding);
            if (it != descriptors.end()) {
                it->second = descriptor;
            } else{
                descriptors.insert(std::make_pair((uint32_t) descriptor.type + descriptor.binding, descriptor));
            }
        };

        inline std::unordered_map<uint32_t, Descriptor> &get_descriptors() {
            return descriptors;
        }

    private:
        std::unordered_map<uint32_t, Descriptor> descriptors;
    };
}

#endif //PATHFINDER_DESCRIPTOR_SET_H
