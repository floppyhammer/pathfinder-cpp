#include "descriptor_set.h"


#include "buffer.h"
#include "texture.h"
#include "../platform.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    void DescriptorSetVk::update_vk_descriptor_set(VkDevice p_device, VkDescriptorSetLayout descriptor_set_layout) {
        device = p_device;

        if (!descriptor_set_allocated) {
            std::vector<VkDescriptorPoolSize> poolSizes{};
            for (auto &d: descriptors) {
                VkDescriptorPoolSize pool_size{};

                if (d.second.type == DescriptorType::UniformBuffer) {
                    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                } else {
                    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                }

                pool_size.descriptorCount = 1;

                poolSizes.push_back(pool_size);
            }

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = 1;

            if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor pool!");
            }

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptor_pool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &descriptor_set_layout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &descriptor_set) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate descriptor sets!");
            }

            descriptor_set_allocated = true;
        }

        std::vector<VkWriteDescriptorSet> descriptor_writes;

        for (auto &pair: descriptors) {
            auto &descriptor = pair.second;

            // Must initialize it with {}.
            VkWriteDescriptorSet descriptor_write{};

            // These two have to be here to prevent them from going out of scope
            // before being used in vkUpdateDescriptorSets().
            VkDescriptorBufferInfo bufferInfo{};
            VkDescriptorImageInfo imageInfo{};

            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = descriptor_set;
            descriptor_write.dstBinding = descriptor.binding;
            descriptor_write.dstArrayElement = 0;
            descriptor_write.descriptorCount = 1;

            if (descriptor.type == DescriptorType::UniformBuffer) {
                auto buffer_vk = static_cast<BufferVk *>(descriptor.buffer.get());

                bufferInfo.buffer = buffer_vk->get_vk_buffer();
                bufferInfo.offset = 0;
                bufferInfo.range = buffer_vk->size;

                descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptor_write.pBufferInfo = &bufferInfo;
            } else if (descriptor.type == DescriptorType::Texture) {
                auto texture_vk = static_cast<TextureVk *>(descriptor.texture.get());

                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = texture_vk->get_image_view();
                imageInfo.sampler = texture_vk->get_sampler();

                descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptor_write.pImageInfo = &imageInfo;
            } else {
                abort();
            }

            vkUpdateDescriptorSets(device,
                                   1,
                                   &descriptor_write,
                                   0,
                                   nullptr);
        }

        // This doesn't work due to using of pBufferInfo and pImageInfo pointers.
//        // Update the contents of a descriptor set object.
//        vkUpdateDescriptorSets(device,
//                               descriptor_writes.size(),
//                               descriptor_writes.data(),
//                               0,
//                               nullptr);
    }

    VkDescriptorSet &DescriptorSetVk::get_vk_descriptor_set() {
        return descriptor_set;
    }

    DescriptorSetVk::~DescriptorSetVk() {
        if (descriptor_set_allocated) {
            // When we destroy the pool, the sets inside are destroyed as well.
            vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
        }
    }
}

#endif
