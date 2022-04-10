//
// Created by floppyhammer on 4/9/2022.
//

#include "descriptor_set.h"

namespace Pathfinder {
    void Pathfinder::DescriptorSet::add_descriptor(const Pathfinder::Descriptor& descriptor) {
        descriptors.insert(std::make_pair((uint32_t) descriptor.type + descriptor.binding, descriptor));
    }
}
