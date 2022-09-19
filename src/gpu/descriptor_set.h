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
        inline void add_or_update_descriptor(const Descriptor &descriptor) {
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
