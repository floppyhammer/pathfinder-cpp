#include "descriptor_set.h"

#include "../platform.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    void DescriptorSetVk::update_vk_descriptor_set() {
        std::vector<VkWriteDescriptorSet> descriptor_writes{};

        for (auto &pair : descriptors) {
            auto &descriptor = pair.second;

            VkWriteDescriptorSet descriptor_write;

            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = descriptor_set;
            descriptor_write.dstBinding = descriptor.binding;
            descriptor_write.dstArrayElement = 0;

            descriptor_write.descriptorCount = 1;

            if (descriptor.type == DescriptorType::UniformBuffer) {
                descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

                VkDescriptorBufferInfo bufferInfo{};
                //bufferInfo.buffer = descriptor.buffer->id;
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(descriptor.buffer->size);

                descriptor_write.pBufferInfo = &bufferInfo;
            } else if (descriptor.type == DescriptorType::Texture) {
                descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
               // imageInfo.imageView = descriptor.texture;
                //imageInfo.sampler = sampler;

                descriptor_write.pImageInfo = &imageInfo;
            }

            descriptor_writes.push_back(descriptor_write);
        }

        // Update the contents of a descriptor set object.
//        vkUpdateDescriptorSets(device,
//                               static_cast<uint32_t>(descriptor_writes.size()),
//                               descriptor_writes.data(),
//                               0,
//                               nullptr);
    }

    VkDescriptorSet &DescriptorSetVk::get_vk_descriptor_set() {
        return descriptor_set;
    }
}

#endif
