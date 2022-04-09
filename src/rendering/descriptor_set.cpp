//
// Created by floppyhammer on 4/9/2022.
//

#include "descriptor_set.h"

namespace Pathfinder {
    void Pathfinder::DescriptorSet::add_descriptor(Pathfinder::Descriptor descriptor) {
        descriptors.insert(std::make_pair(descriptor.binding, descriptor));
    }
}
