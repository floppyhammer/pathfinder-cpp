#include "descriptor_set.h"

#include "buffer.h"
#include "texture.h"

namespace Pathfinder {

void DescriptorSetVk::update_vk_descriptor_set(VkDevice vk_device, VkDescriptorSetLayout vk_descriptor_set_layout) {
    vk_device_ = vk_device;

    // Create descriptor pool and allocate descriptor sets.
    if (!descriptor_set_allocated_) {
        // Get pool sizes.
        std::vector<VkDescriptorPoolSize> poolSizes{};

        for (auto &d : descriptors) {
            VkDescriptorPoolSize pool_size{};
            pool_size.descriptorCount = 1;

            switch (d.second.type) {
                case DescriptorType::UniformBuffer: {
                    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                } break;
                case DescriptorType::Sampler: {
                    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                } break;
                case DescriptorType::StorageBuffer: {
                    pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                } break;
                case DescriptorType::Image: {
                    pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                } break;
                default: {
                    abort();
                }
            }

            poolSizes.push_back(pool_size);
        }

        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        pool_info.pPoolSizes = poolSizes.data();
        pool_info.maxSets = 1;

        if (vkCreateDescriptorPool(vk_device_, &pool_info, nullptr, &vk_descriptor_pool_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }

        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = vk_descriptor_pool_;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &vk_descriptor_set_layout;

        if (vkAllocateDescriptorSets(vk_device_, &alloc_info, &vk_descriptor_set_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }

        descriptor_set_allocated_ = true;
    }

    for (auto &pair : descriptors) {
        auto &descriptor = pair.second;

        // Must initialize it with {}.
        VkWriteDescriptorSet descriptor_write{};

        // These two have to be here to prevent them from going out of scope
        // before being used in vkUpdateDescriptorSets().
        VkDescriptorBufferInfo buffer_info{};
        VkDescriptorImageInfo image_info{};

        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = vk_descriptor_set_;
        descriptor_write.dstBinding = descriptor.binding;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorCount = 1;

        switch (descriptor.type) {
            case DescriptorType::UniformBuffer:
            case DescriptorType::StorageBuffer: {
                // It's possible to bind nothing at a slot in the layout.
                // However, you may see validation errors.
                if (descriptor.buffer == nullptr) {
                    continue;
                }

                auto buffer_vk = static_cast<BufferVk *>(descriptor.buffer.get());

                buffer_info.buffer = buffer_vk->get_vk_buffer();
                buffer_info.offset = 0;
                buffer_info.range = buffer_vk->get_size();

                if (descriptor.type == DescriptorType::UniformBuffer) {
                    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                } else if (descriptor.type == DescriptorType::StorageBuffer) {
                    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                }

                descriptor_write.pBufferInfo = &buffer_info;
            } break;
            case DescriptorType::Sampler:
            case DescriptorType::Image: {
                // It's possible to bind nothing at a slot in the layout.
                // However, you may see validation errors.
                if (descriptor.texture == nullptr) {
                    continue;
                }

                auto texture_vk = static_cast<TextureVk *>(descriptor.texture.get());
                auto sampler_vk = static_cast<SamplerVk *>(descriptor.sampler.get());

                image_info.imageView = texture_vk->get_image_view();

                if (descriptor.type == DescriptorType::Sampler) {
                    image_info.sampler = sampler_vk->get_sampler();
                    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                } else if (descriptor.type == DescriptorType::Image) {
                    image_info.sampler = VK_NULL_HANDLE;
                    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                }

                descriptor_write.pImageInfo = &image_info;
            } break;
            default:
                abort();
        }

        // If you want to update multiple descriptor sets at once,
        // make sure the pBufferInfo or pImageInfo pointer is still valid at this point.
        vkUpdateDescriptorSets(vk_device_, 1, &descriptor_write, 0, nullptr);
    }
}

VkDescriptorSet &DescriptorSetVk::get_vk_descriptor_set() {
    return vk_descriptor_set_;
}

DescriptorSetVk::~DescriptorSetVk() {
    if (descriptor_set_allocated_) {
        // When we destroy the pool, the sets inside are destroyed as well.
        vkDestroyDescriptorPool(vk_device_, vk_descriptor_pool_, nullptr);
    }
}

} // namespace Pathfinder
