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
        GeneralBuffer = 20,
        Texture = 40,
        Image = 50,
        Max = 70,
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
        void add_descriptor(const Descriptor& descriptor);

        std::unordered_map<uint32_t, Descriptor> descriptors;
    };
}

#endif //PATHFINDER_DESCRIPTOR_SET_H
